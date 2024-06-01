from database import *
from fastapi import *
from fastapi.security import *
from fastapi.staticfiles import *
from fastapi.responses import *

from fastapi_mqtt import MQTTConfig, FastMQTT
from sqlalchemy.orm import sessionmaker, Session
from dataclasses import dataclass

from sqlalchemy import func
import sqlalchemy

import bcrypt
import jwt
from datetime import datetime, timedelta

import json
import ssl

app = FastAPI()

mqtt_config = MQTTConfig(
    host="broker.emqx.io",
    port=8883,
    ssl=ssl.create_default_context(cafile="broker.emqx.io-ca.crt")
)
fast_mqtt = FastMQTT(config=mqtt_config)
fast_mqtt.init_app(app)

app.mount("/static", StaticFiles(directory="static", html=True), name="static")

#######################################################################################
# CRYPTO
#######################################################################################

ACCESS_TOKEN_EXPIRE_MINUTES = 30
SECRET_KEY = "asdasd"
ALGORITHM = "HS256"

def hash_password(password: str) -> str:
	return bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt()).decode('utf-8')

def verify_password(plain_password: str, hashed_password: str) -> bool:
	return bcrypt.checkpw(plain_password.encode('utf-8'), hashed_password.encode('utf-8'))

def create_access_token(data: dict, expires_delta: timedelta = None):
	to_encode = data.copy()
	if expires_delta:
		expire = datetime.utcnow() + expires_delta
	else:
		expire = datetime.utcnow() + timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
	to_encode.update({"exp": expire})
	encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
	return encoded_jwt

def decode_access_token(token: str):
	try:
		payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])
		return payload
	except jwt.PyJWTError:
		return None

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

#######################################################################################
# DB
#######################################################################################

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=db_engine)

def get_db():
	db = SessionLocal()
	try:
		yield db
	finally:
		db.close()

def get_user_by_username(db: Session, username: str):
	return db.query(User).filter(User.username == username).first()

def get_device_by_device_id(db: Session, device_id: str):
	return db.query(Device).filter(Device.device_id == device_id).first()

def get_devices_by_user(db: Session, user: User):
	return db.query(Device).filter(Device.user == user).all()

@dataclass
class UserCreateJSON():
	username: str
	password: str

@dataclass
class VisitorsRqJSON():
	from_utc_ts: int

@dataclass
class AddDeviceFORM():
	device_id: str = Form(...)
	userlabel: str = Form(...)

@dataclass
class DeviceSettingsFORM():
	userlabel: str = Form(...)
	entrance: str = Form(...)

#######################################################################################
# Registration, cookie based authentication
#######################################################################################

def get_current_user(request: Request, db: Session = Depends(get_db)):
	token = request.cookies.get("access_token")

	if not token:
		raise HTTPException(401, "Token missing")

	payload = decode_access_token(token)

	if payload is None:
		raise HTTPException(401, "Access denied")

	username = payload.get("sub")

	if username is None:
		raise HTTPException(401, "Access denied")

	user = get_user_by_username(db, username)

	if not user:
		raise HTTPException(401, "??")

	return user

@app.get("/")
def index():
	return FileResponse("index.html")

@app.post("/register")
def register_user(user: UserCreateJSON, db: Session = Depends(get_db)):
	db_user = get_user_by_username(db, user.username)

	if db_user:
		raise HTTPException(400, "Username taken")

	hashed_password = hash_password(user.password)
	new_user = User(username=user.username, password=hashed_password)
	db.add(new_user)
	db.commit()
	db.refresh(new_user)

	return {"status": "ok"}

@app.post("/login")
def login(response: Response, username: str = Form(), password: str = Form(), db: Session = Depends(get_db)):
	user = get_user_by_username(db, username)

	if not user:
		raise HTTPException(401, "Username not known")

	if not verify_password(password, user.password):
		raise HTTPException(401, "Wrong password")

	access_token_expires = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
	access_token = create_access_token(
		data={"sub": user.username}, expires_delta=access_token_expires
	)

	response.set_cookie("access_token", access_token, httponly=True)
	response.headers["Location"] = "/"

	return {"status": "ok"}

@app.post("/logout")
def logout(response: Response):
	response.delete_cookie("access_token")
	response.headers["Location"] = "/"

	return {"status": "ok"}

@app.get("/users/me")
def read_users_me(current_user: User = Depends(get_current_user)):
	return {"username": current_user.username}

#######################################################################################
# Device registration, relinquishment, list
#######################################################################################

@app.post("/add_device")
def add_device(params: AddDeviceFORM = Depends(), current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	device_id = params.device_id
	userlabel = params.userlabel

	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found, make sure it is powered and connected to a network")

	if device.user is not None:
		if device.user == current_user:
			raise HTTPException(400, "Device already assigned to you")
		else:	
			raise HTTPException(400, "Device already assigned to somebody else")

	device.user = current_user
	device.userlabel = userlabel
	db.commit()

	return {"status": "ok"}

@app.post("/list_devices")
def list_devices(current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	devices = get_devices_by_user(db, current_user)

	device_list = [
		{
			"device_id": x.device_id,
			"userlabel": x.userlabel,
			"last_seen_healthy": x.last_seen_healthy.isoformat().replace("T", "Z")
		}
	for x in devices]

	return device_list

@app.get("/devices/{device_id}")
def device_info(device_id: str, current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found")

	if device.user != current_user:
		raise HTTPException(401, "Access denied")

	data = db.query(VIS4Message).filter(VIS4Message.device == device).order_by(VIS4Message.id.desc()).first()

	return {
		"device_id": device.device_id,
		"userlabel": device.userlabel,
		"last_seen_healthy": device.last_seen_healthy.isoformat().replace("T", "Z"),
		"direction_of_entrance": device.direction_of_entrance
	}

@app.post("/devices/{device_id}/visitors")
def device_visitors(device_id: str, params:VisitorsRqJSON, current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found")

	if device.user != current_user:
		raise HTTPException(401, "Access denied")

	data = db.query(VIS4Message).filter(VIS4Message.device == device).group_by(VIS4Message.time >= datetime.fromtimestamp(params.from_utc_ts)).having(func.max(VIS4Message.time)).all()

	if len(data) == 2:
		a = data[0]
		b = data[1]
		ut = b.u - a.u
		dt = b.d - a.d
		lt = b.l - a.l
		rt = b.r - a.r
	elif len(data) == 1:
		a = data[0]
		b = data[0]
		ut = b.u
		dt = b.d
		lt = b.l
		rt = b.r
	else:
		raise HTTPException(400, "??")

	directions = {
		"u": { "today": ut, "total": b.u },
		"d": { "today": dt, "total": b.d },
		"l": { "today": lt, "total": b.l },
		"r": { "today": rt, "total": b.r },
	}

	return directions[device.direction_of_entrance]

@app.post("/devices/{device_id}/visitors_full")
def device_visitors_full(device_id: str, params:VisitorsRqJSON, current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found")

	if device.user != current_user:
		raise HTTPException(401, "Access denied")

	data = db.query(VIS4Message).filter(VIS4Message.device == device).group_by(VIS4Message.time >= datetime.fromtimestamp(params.from_utc_ts)).having(func.max(VIS4Message.time)).all()
	first_data = db.query(VIS4Message).filter(VIS4Message.device == device).first()

	if len(data) == 2:
		a = data[0]
		b = data[1]
	elif len(data) == 1:
		a = first_data
		b = data[0]
	else:
		raise HTTPException(400, "??")

	ut = b.u - a.u
	dt = b.d - a.d
	lt = b.l - a.l
	rt = b.r - a.r

	return {
		"recent": {
			"since": a.time.isoformat().replace("T", "Z"),
			"until": b.time.isoformat().replace("T", "Z"),
			"u": ut,
			"d": dt,
			"l": lt,
			"r": rt
		},
		"total": {
			"since": first_data.time.isoformat().replace("T", "Z"),
			"u": b.u,
			"d": b.d,
			"l": b.l,
			"r": b.r,
		}
	}

@app.post("/devices/{device_id}/settings")
def device_settings(device_id: str, params: DeviceSettingsFORM = Depends(), current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	userlabel = params.userlabel
	direction_of_entrance = params.entrance

	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found")

	if device.user != current_user:
		raise HTTPException(401, "Access denied")

	device.userlabel = userlabel
	device.direction_of_entrance = direction_of_entrance
	db.commit()

	return {"status": "ok"}

@app.get("/devices/{device_id}/remove")
def device_remove(device_id: str, current_user: User = Depends(get_current_user), db: Session = Depends(get_db)):
	device = get_device_by_device_id(db, device_id)

	if device is None:
		raise HTTPException(400, "Device not found")

	if device.user != current_user:
		raise HTTPException(401, "Access denied")

	device.user = None
	db.commit()

	sqlalchemy.delete(VIS4Message).where(VIS4Message.device == device)
	db.commit()

	return {"status": "ok"}
	

#######################################################################################
# MQTT
#######################################################################################

@fast_mqtt.subscribe("axkuhta/+/visitors4")
async def handle_visitors4_data(client, topic, payload, qos, properties, db: Session = SessionLocal()):
	try:
		payload_json = json.loads(payload.decode())

		device_time = datetime.fromisoformat( payload_json["device_timestamp"] )
		runtime_random_id = payload_json["runtime_random_id"]
		u = payload_json["u"]
		d = payload_json["d"]
		l = payload_json["l"]
		r = payload_json["r"]

	except Exception as e:
		print(f"[Visitors4] invalid data from {topic}: {e}")

	device_id = topic.split("/")[-2]

	device = get_device_by_device_id(db, device_id)

	if device is None:
		device = Device(device_id=device_id, userlabel="Unlabeled", direction_of_entrance="r", last_seen_healthy=datetime.utcnow())
		db.add(device)

		print(f"[Visitors4] new device online: {device_id}")
	else:
		device.last_seen_healthy = datetime.utcnow()

	if device.user is not None:
		message = VIS4Message(runtime_random_id=runtime_random_id, time=device_time, u=u, d=d, l=l, r=r)
		message.device = device
		db.add(message)

	db.commit()
	db.close()

# uvicorn main:app --reload --timeout-keep-alive 30
