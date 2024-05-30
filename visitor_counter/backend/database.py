from sqlalchemy import create_engine, Column, Integer, String, Float, TIMESTAMP, ForeignKey, Boolean
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship

db_engine = create_engine('sqlite:///service.db', echo=False)
Base = declarative_base()


class User(Base):
	__tablename__ = "users"

	id = Column(Integer, primary_key=True, autoincrement=True)
	username = Column(String(255), nullable=False, unique=True)
	password = Column(String(255), nullable=False)
	firstname = Column(String(255))
	lastname = Column(String(255))
	devices = relationship("Device", back_populates="user")
	has_device = Column(Boolean, default=False)


class Device(Base):
	__tablename__ = "devices"

	id = Column(Integer, primary_key=True, autoincrement=True)
	device_id = Column(String(255), nullable=False, unique=True)
	userlabel = Column(String(255), nullable=False)
	last_seen_healthy = Column(TIMESTAMP, nullable=False)
	user_id = Column(Integer, ForeignKey('users.id'))
	user = relationship("User", back_populates="devices")
	vis4messages = relationship("VIS4Message", back_populates="device")

class VIS4Message(Base):
	__tablename__ = 'vis4messages'

	id = Column(Integer, primary_key=True, autoincrement=True)
	device_id = Column(Integer, ForeignKey('devices.id'), nullable=False)
	runtime_random_id = Column(Integer, nullable=False)
	time = Column(TIMESTAMP, nullable=False)
	u = Column(Integer, nullable=False)
	d = Column(Integer, nullable=False)
	l = Column(Integer, nullable=False)
	r = Column(Integer, nullable=False)

	device = relationship("Device", back_populates="vis4messages")

Base.metadata.create_all(bind=db_engine)
