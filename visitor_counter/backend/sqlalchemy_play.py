from sqlalchemy.orm import sessionmaker, Session
from database import *

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=db_engine)

db = SessionLocal()

print("################################ ALL DEVICES ################################")

for device in db.query(Device).all():
	print(device.device_id, device.last_seen_healthy, device.user)
