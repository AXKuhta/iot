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
	name = Column(String(255), nullable=False, unique=True)
	latitude = Column(Float, nullable=False)
	longitude = Column(Float, nullable=False)
	is_active = Column(Boolean, nullable=False, default=False)
	user_id = Column(Integer, ForeignKey('users.id'))
	user = relationship("User", back_populates="devices")
	amessages = relationship("AMessage", back_populates="device")

class AMessage(Base):
	__tablename__ = 'amessages'

	id = Column(Integer, primary_key=True, autoincrement=True)
	device_id = Column(Integer, ForeignKey('devices.id'), nullable=False)
	time = Column(TIMESTAMP, nullable=False)
	u = Column(Integer, nullable=False)
	d = Column(Integer, nullable=False)
	l = Column(Integer, nullable=False)
	r = Column(Integer, nullable=False)

	device = relationship("Device", back_populates="amessages")

Base.metadata.create_all(bind=db_engine)
