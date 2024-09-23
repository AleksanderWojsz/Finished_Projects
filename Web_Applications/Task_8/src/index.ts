let global_id: number = 0;
let toDeleteId: number = -1;
let isMousePressed = false;
let newRect: Rectangle;
let startX: number = 0;
let startY: number = 0;

class Rectangle {
    x1: number;
    y1: number;
    x2: number;
    y2: number;
    color: string;
    id: number;

    constructor(x1: number, y1: number, x2: number, y2: number, color: string) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
        this.color = color;
        this.id = global_id++;
    }
}

class SingleImage {
    rectangles: Rectangle[] = [];

    addRectangle(rectangle: Rectangle) {
        this.rectangles.push(rectangle);
    }

    deleteRectangleById(id: number) {
        const index = this.rectangles.findIndex(rect => rect.id === id);
        this.rectangles.splice(index, 1);
    }

    toJson(): string {
        return JSON.stringify(this)
    }
}

const singleImage = new SingleImage();


// Renderowanie i dodawanie

function renderBackground() {
    const svgBackground = document.getElementById('space_to_render_image') as unknown as SVGSVGElement;
    svgBackground.innerHTML = ''; // resetowanie obrazka

    singleImage.rectangles.forEach(function (rectangle) {
        const rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        rect.setAttribute('x', Math.min(rectangle.x1, rectangle.x2).toString());
        rect.setAttribute('y', Math.min(rectangle.y1, rectangle.y2).toString());
        rect.setAttribute('width', Math.abs(rectangle.x2 - rectangle.x1).toString());
        rect.setAttribute('height', Math.abs(rectangle.y2 - rectangle.y1).toString());
        rect.setAttribute('fill', rectangle.color);
        rect.setAttribute('id', rectangle.id.toString());
        // rect.addEventListener('click', () => {
        //     const rectId = rect.getAttribute('id');
        //     singleImage.deleteRectangleById(rectId);
        //     renderBackground();
        //     console.log(`Usuniety ${rectId}`)
        // });
        svgBackground.appendChild(rect);
    });
}

function addRectangle() {
    const x1 = parseInt((document.getElementById('x1') as HTMLInputElement).value);
    const y1 = parseInt((document.getElementById('y1') as HTMLInputElement).value);
    const x2 = parseInt((document.getElementById('x2') as HTMLInputElement).value);
    const y2 = parseInt((document.getElementById('y2') as HTMLInputElement).value);
    const color = (document.getElementById('pick_color') as HTMLInputElement).value;
    singleImage.addRectangle(new Rectangle(x1, y1, x2, y2, color));
    renderBackground();
}


// Dodawanie i usuwanie myszką

function onMouseDown(event: MouseEvent) {
    startX = event.offsetX;
    startY = event.offsetY;
    isMousePressed = true;
    newRect = new Rectangle(startX, startY, startX, startY, (document.getElementById('pick_color') as HTMLInputElement).value);
}

function onMouseMove(event: MouseEvent) {
    if (isMousePressed) {
        newRect.x2 = event.offsetX;
        newRect.y2 = event.offsetY;

        renderBackground();

        const svgBackground = document.getElementById('space_to_render_image') as any as SVGSVGElement;
        const rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        rect.setAttribute('x', Math.min(newRect.x1, newRect.x2).toString());
        rect.setAttribute('y', Math.min(newRect.y1, newRect.y2).toString());
        rect.setAttribute('width', Math.abs(newRect.x2 - newRect.x1).toString());
        rect.setAttribute('height', Math.abs(newRect.y2 - newRect.y1).toString());
        rect.setAttribute('fill', newRect.color);
        svgBackground.appendChild(rect);
    }
}

function onMouseUp(event: MouseEvent) {
    isMousePressed = false;
    newRect.x2 = event.offsetX;
    newRect.y2 = event.offsetY;

    if (Math.abs(newRect.x1 - newRect.x2) >= 1 && Math.abs(newRect.y1 - newRect.y2) >= 1) {
        singleImage.addRectangle(newRect);
    } else {

        // Bezposrednio znajdujemy obrazek do usuniecia
        // Od konca zeby znalezc najnowszy
        for (let i = singleImage.rectangles.length - 1; i >= 0; i--) {
            const rect = singleImage.rectangles[i];
            if (startX >= Math.min(rect.x1, rect.x2) &&
                startX <= Math.max(rect.x1, rect.x2) &&
                startY >= Math.min(rect.y1, rect.y2) &&
                startY <= Math.max(rect.y1, rect.y2)) {

                let toDelete: Rectangle = singleImage.rectangles[i];
                toDeleteId = toDelete.id;

                // @ts-ignore
                document.getElementById("x1_del").setAttribute('value', `${toDelete.x1}`);
                // @ts-ignore
                document.getElementById("y1_del").setAttribute('value', `${toDelete.y1}`);
                // @ts-ignore
                document.getElementById("x2_del").setAttribute('value', `${toDelete.x2}`);
                // @ts-ignore
                document.getElementById("y2_del").setAttribute('value', `${toDelete.y2}`);
                // @ts-ignore
                document.getElementById("color_del").setAttribute('value', `${toDelete.color}`);
                return;
            }
        }

        console.log("Nie ma takiego prostokata");
    }

    renderBackground();
}

function deleteSelectedRectangle() {
    if (toDeleteId != -1) {
        singleImage.deleteRectangleById(toDeleteId);
        toDeleteId = -1;
        renderBackground();
    } else {
        console.log("Nie wybrano prostokata");
    }
}



