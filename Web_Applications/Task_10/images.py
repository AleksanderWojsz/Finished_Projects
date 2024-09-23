from fastapi import FastAPI, Depends, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from sqlalchemy.orm import Session
import models
from database import SessionLocal, engine

app = FastAPI()

models.Base.metadata.create_all(bind=engine)

websockets_list = []

def get_db():
    try:
        db = SessionLocal()
        yield db
    finally:
        db.close()

class RectangleData(BaseModel):
    x1: int = 0
    y1: int = 0
    x2: int = 100
    y2: int = 100
    color: str = "red"

class ImageCreate(BaseModel):
    title: str
    tags: set
    rectangles: list[RectangleData]

@app.post("/images")
async def create_image(image_data: ImageCreate, db: Session = Depends(get_db)):
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

    json = {
        "id": new_image.id,
        "title": new_image.title,
        "tags": [tag.name for tag in new_image.tags],
        "rectangles": [{
            "x1": rect.x1, "y1": rect.y1, "x2": rect.x2, "y2": rect.y2, "color": rect.color
        } for rect in new_image.rectangles]
    }

    for websocket in websockets_list:
        await websocket.send_json(json)

    return json

@app.websocket("/ws/images")
async def websocket_images(websocket: WebSocket, db: Session = Depends(get_db)):
    await websocket.accept()

    global websockets_list
    websockets_list.append(websocket)

    all_images = db.query(models.Image).all()
    for image in all_images: # Wysyłanie już istniejących prostokątów. Każdy osobno
        rectangles_data = [
            {"id": rectangle.id, "x1": rectangle.x1, "y1": rectangle.y1, "x2": rectangle.x2, "y2": rectangle.y2, "color": rectangle.color}
            for rectangle in image.rectangles
        ]
        response_data = {
            "id": image.id,
            "title": image.title,
            "tags": [tag.name for tag in image.tags],
            "rectangles": rectangles_data
        }

        await websocket.send_json(response_data)

    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect: # Pythonowy odpowiednik try catch
        websockets_list.remove(websocket)