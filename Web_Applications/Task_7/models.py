from sqlalchemy import Column, Integer, String, Table, ForeignKey
from sqlalchemy.orm import relationship
from database import Base

# Tabela asocjacyjna https://stackoverflow.com/a/68633850/22553511
image_tag_table = Table('image_tag', Base.metadata,
    Column('image_id', ForeignKey('images.id'), primary_key=True),
    Column('tag_id', ForeignKey('tags.id'), primary_key=True))


class Rectangle(Base):
    __tablename__ = 'rectangles'
    id = Column(Integer, primary_key=True)
    x1 = Column(Integer)
    y1 = Column(Integer)
    x2 = Column(Integer)
    y2 = Column(Integer)
    color = Column(String)
    image_id = Column(Integer, ForeignKey('images.id'))

    image = relationship("Image", back_populates="rectangles")

class Image(Base):
    __tablename__ = 'images'
    id = Column(Integer, primary_key=True)
    title = Column(String)
    tags = relationship('Tag', secondary=image_tag_table, back_populates='images')
    rectangles = relationship('Rectangle', back_populates='image', cascade="all, delete, delete-orphan") # cascade takie, żeby wszystkie prostokąty były usuwane wraz z obrazkiem

class Tag(Base):
    __tablename__ = 'tags'
    id = Column(Integer, primary_key=True)
    name = Column(String, unique=True)
    images = relationship('Image', secondary=image_tag_table, back_populates='tags')
