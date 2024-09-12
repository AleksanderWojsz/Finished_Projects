import asyncio
import random
from typing import List

from fastapi import FastAPI, HTTPException, Depends, Query
from pydantic import BaseModel
from sqlalchemy.orm import Session
import models
from database import SessionLocal, engine

app = FastAPI(
    title="Zadanie aww7",
    version="0.1",
    contact={
        "name": "Aleksander Wojsz",
        "url": "https://github.com/AleksanderWojsz/aww7",
    },
    license_info={
        "name": "MIT",
        "url": "https://opensource.org/licenses/MIT",
    },
)

models.Base.metadata.create_all(bind=engine)


def get_db():
    try:
        db = SessionLocal()
        yield db
    finally:
        db.close()


@app.get("/tags")
def list_tags(db: Session = Depends(get_db)):
    """
    1. end-point listujący wszystkie tagi wraz z liczbą obrazków przypisanych do danego tagu.
    """
    tags = db.query(models.Tag).all()
    return [{"tag": tag.name, "number_of_images": len(tag.images)} for tag in tags]


@app.get("/images")
def list_images(db: Session = Depends(get_db)):
    """
    2. end-point listujący wszystkie obrazki wraz z ich tagami
    """
    images = db.query(models.Image).all()
    return [{"id": image.id, "title": image.title, "tags": [tag.name for tag in image.tags]} for image in images]


@app.get("/images/{tag_name}")
def images_by_tag(tag_name: str, db: Session = Depends(get_db)):
    """
    3. end-point listujący wszystkie obrazki przypisane do danego tagu
    """
    tag = db.query(models.Tag).filter(models.Tag.name == tag_name).first()
    return [{"id": image.id, "title": image.title, "tags": [tag.name for tag in image.tags]} for image in tag.images]


@app.delete("/images/del")
def delete_images(image_ids: list[int], db: Session = Depends(get_db)):
    """
    4. end-point kasujący obrazki
    """
    db.query(models.Image).filter(models.Image.id.in_(image_ids)).delete()
    db.commit()
    return {"result": "Images deleted"}





@app.get("/count")
def count_images(db: Session = Depends(get_db)):
    return {"number_of_images": db.query(models.Image).count()}

#
# @app.get("/images/get/{image_id}")
# def get_image(image_id: int, db: Session = Depends(get_db)):
#     image = db.query(models.Image).filter(models.Image.id == image_id).first()
#     rectangles_data = [
#         {"id": rectangle.id, "x1": rectangle.x1, "y1": rectangle.y1, "x2": rectangle.x2, "y2": rectangle.y2,
#          "color": rectangle.color} for rectangle in image.rectangles
#     ]
#
#     return {
#         "id": image.id,
#         "title": image.title,
#         "tags": [tag.name for tag in image.tags],
#         "rectangles": rectangles_data
#     }


@app.get("/images/get/{image_id}")
async def get_image(image_id: int, db: Session = Depends(get_db)):

    if random.choice([True, False, False]):
        raise HTTPException(status_code=500, detail="Internal server error")

    if random.choice([True, False, False]):
        await asyncio.sleep(2)

    image = db.query(models.Image).filter(models.Image.id == image_id).first()

    rectangles_data = [
        {"id": rectangle.id, "x1": rectangle.x1, "y1": rectangle.y1, "x2": rectangle.x2, "y2": rectangle.y2,
         "color": rectangle.color} for rectangle in image.rectangles
    ]

    # if random.choice([True, False, False, False]):
    #     # Generowanie listy 100 prostokątów z losowymi wartościami
    #     large_data = [{
    #         "id": idx,
    #         "x1": random.randint(0, 100),
    #         "y1": random.randint(0, 100),
    #         "x2": random.randint(0, 100),
    #         "y2": random.randint(0, 100),
    #         "color": random.choice(["red", "green", "blue", "yellow", "pink"])
    #     } for idx in range(1000)]
    #     return {"id": image.id, "title": image.title, "tags": [tag.name for tag in image.tags], "rectangles": large_data}


    return {
        "id": image.id,
        "title": image.title,
        "tags": [tag.name for tag in image.tags],
        "rectangles": rectangles_data
    }







# Niewymagane dodawanie obrazków
class RectangleData(BaseModel):
    x1: int = 0
    y1: int = 0
    x2: int = 100
    y2: int = 100
    color: str = "red"


class ImageCreate(BaseModel):
    title: str
    tags: set
    rectangles: List[RectangleData]

@app.post("/images")
def create_image(image_data: ImageCreate, db: Session = Depends(get_db)):
    new_image = models.Image(title=image_data.title)

    for tag_name in image_data.tags:
        tag = db.query(models.Tag).filter(models.Tag.name == tag_name).first()
        if not tag:
            tag = models.Tag(name=tag_name)
            db.add(tag)
        new_image.tags.append(tag)

    for rect_data in image_data.rectangles:
        new_rectangle = models.Rectangle(
            x1=rect_data.x1, y1=rect_data.y1, x2=rect_data.x2, y2=rect_data.y2, color=rect_data.color
        )
        new_image.rectangles.append(new_rectangle)

    db.add(new_image)
    db.commit()
    return {
        "id": new_image.id,
        "title": new_image.title,
        "tags": [tag.name for tag in new_image.tags],
        "rectangles": [{
            "x1": rect.x1, "y1": rect.y1, "x2": rect.x2, "y2": rect.y2, "color": rect.color
        } for rect in new_image.rectangles]
    }
