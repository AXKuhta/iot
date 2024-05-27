from database import *
from fastapi import *
from fastapi.security import *
from fastapi.staticfiles import *
from fastapi.responses import *

from fastapi_mqtt import MQTTConfig, FastMQTT
from sqlalchemy.orm import sessionmaker, Session
from pydantic import BaseModel

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

class UserCreateJSON(BaseModel):
    username: str
    password: str

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
# MQTT
#######################################################################################

@fast_mqtt.subscribe("axkuhta/+/sensors")
async def collect_data(client, topic, payload, qos, properties, db: Session = Depends(get_db)):
	try:
		payload_json = json.loads(payload.decode())

		device_time = payload_json["device_timestamp"]
		u = payload_json["visitors_u"]
		d = payload_json["visitors_d"]
		l = payload_json["visitors_l"]
		r = payload_json["visitors_r"]

		AMessage(time=device_time, u=u, d=d, l=l, r=r)

		"""
		with sessionmaker(bind=db_engine)() as session:
		device = session.query(Device).filter(Device.name == device_name).first()
		if device is not None:
		record = Record(device_id=device.id, temperature=temperature, humidity=humidity,
		radioactivity=radioactivity, pm25=pm25, pm10=pm10, noisiness=noisiness,
		time=datetime.strptime(timestamp, "%Y-%m-%dT%H:%M:%S.%f"))
		session.add(record)
		device.is_active = True
		session.commit()
		"""
	except Exception as e:
		print(f"[Data] invalid data from {topic}: {e}")

# uvicorn main:app --reload
