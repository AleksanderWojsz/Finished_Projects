var global_id = 0;
var toDeleteId = -1;
var isMousePressed = false;
var newRect;
var startX = 0;
var startY = 0;
var Rectangle = /** @class */ (function () {
    function Rectangle(x1, y1, x2, y2, color) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
        this.color = color;
        this.id = global_id++;
    }
    return Rectangle;
}());
var SingleImage = /** @class */ (function () {
    function SingleImage() {
        this.rectangles = [];
    }
    SingleImage.prototype.addRectangle = function (rectangle) {
        this.rectangles.push(rectangle);
    };
    SingleImage.prototype.deleteRectangleById = function (id) {
        var index = this.rectangles.findIndex(function (rect) { return rect.id === id; });
        this.rectangles.splice(index, 1);
    };
    SingleImage.prototype.toJson = function () {
        return JSON.stringify(this);
    };
    return SingleImage;
}());
var singleImage = new SingleImage();
// Renderowanie i dodawanie
function renderBackground() {
    var svgBackground = document.getElementById('space_to_render_image');
    svgBackground.innerHTML = ''; // resetowanie obrazka
    singleImage.rectangles.forEach(function (rectangle) {
        var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
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
    var x1 = parseInt(document.getElementById('x1').value);
    var y1 = parseInt(document.getElementById('y1').value);
    var x2 = parseInt(document.getElementById('x2').value);
    var y2 = parseInt(document.getElementById('y2').value);
    var color = document.getElementById('pick_color').value;
    singleImage.addRectangle(new Rectangle(x1, y1, x2, y2, color));
    renderBackground();
}
// Dodawanie i usuwanie myszką
function onMouseDown(event) {
    startX = event.offsetX;
    startY = event.offsetY;
    isMousePressed = true;
    newRect = new Rectangle(startX, startY, startX, startY, document.getElementById('pick_color').value);
}
function onMouseMove(event) {
    if (isMousePressed) {
        newRect.x2 = event.offsetX;
        newRect.y2 = event.offsetY;
        renderBackground();
        var svgBackground = document.getElementById('space_to_render_image');
        var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        rect.setAttribute('x', Math.min(newRect.x1, newRect.x2).toString());
        rect.setAttribute('y', Math.min(newRect.y1, newRect.y2).toString());
        rect.setAttribute('width', Math.abs(newRect.x2 - newRect.x1).toString());
        rect.setAttribute('height', Math.abs(newRect.y2 - newRect.y1).toString());
        rect.setAttribute('fill', newRect.color);
        svgBackground.appendChild(rect);
    }
}
function onMouseUp(event) {
    isMousePressed = false;
    newRect.x2 = event.offsetX;
    newRect.y2 = event.offsetY;
    if (Math.abs(newRect.x1 - newRect.x2) >= 1 && Math.abs(newRect.y1 - newRect.y2) >= 1) {
        singleImage.addRectangle(newRect);
    }
    else {
        // Bezposrednio znajdujemy obrazek do usuniecia
        // Od konca zeby znalezc najnowszy
        for (var i = singleImage.rectangles.length - 1; i >= 0; i--) {
            var rect = singleImage.rectangles[i];
            if (startX >= Math.min(rect.x1, rect.x2) &&
                startX <= Math.max(rect.x1, rect.x2) &&
                startY >= Math.min(rect.y1, rect.y2) &&
                startY <= Math.max(rect.y1, rect.y2)) {
                var toDelete = singleImage.rectangles[i];
                toDeleteId = toDelete.id;
                // @ts-ignore
                document.getElementById("x1_del").setAttribute('value', "".concat(toDelete.x1));
                // @ts-ignore
                document.getElementById("y1_del").setAttribute('value', "".concat(toDelete.y1));
                // @ts-ignore
                document.getElementById("x2_del").setAttribute('value', "".concat(toDelete.x2));
                // @ts-ignore
                document.getElementById("y2_del").setAttribute('value', "".concat(toDelete.y2));
                // @ts-ignore
                document.getElementById("color_del").setAttribute('value', "".concat(toDelete.color));
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
    }
    else {
        console.log("Nie wybrano prostokata");
    }
}
