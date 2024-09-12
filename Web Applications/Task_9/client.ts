import axios from 'axios';

interface ImageResponse {
    id: number;
    title: string;
    tags: string[];
    rectangles: {
        id: number;
        x1: number;
        y1: number;
        x2: number;
        y2: number;
        color: string;
    }[];
}

async function getImage(imageId: number) {
    try {
        let response = await axios.get(`http://localhost:8000/images/get/${imageId}`);

        if (JSON.stringify(response.data).length > 1000) { // sprawdzenie długości
            return new Error('Image size is too large');
        }

        return response.data;
    } catch (error) {
        return new Error('Server error');
    }
}

async function getImagesIdList() {
    let response = await axios.get(`http://localhost:8000/images`);
    return response.data.map((image: { id: number; title: string; tags: string[] }) => Number(image.id)); // https://stackoverflow.com/a/71257991/22553511
}

async function displayImages() {
    let image_list = document.getElementById('images_list') as HTMLDivElement;
    let id_list: number[] = await getImagesIdList();

    for (let id of id_list) {
        let space_for_image = document.createElement('div');
        space_for_image.className = 'space_for_image';

        loadImage(id, space_for_image);

        image_list.appendChild(space_for_image);
    }
}

function renderImageSVG(imageData: ImageResponse, container: HTMLElement) {
    let svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");

    imageData.rectangles.forEach(rectangle => {
        let rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        rect.setAttribute('x', Math.min(rectangle.x1, rectangle.x2).toString());
        rect.setAttribute('y', Math.min(rectangle.y1, rectangle.y2).toString());
        rect.setAttribute('width', Math.abs(rectangle.x2 - rectangle.x1).toString());
        rect.setAttribute('height', Math.abs(rectangle.y2 - rectangle.y1).toString());
        rect.setAttribute('fill', rectangle.color);
        svg.appendChild(rect);
    });

    container.innerHTML = '';
    container.appendChild(svg);
}

function loadImage(imageId: number, container: HTMLElement) {
    container.innerHTML = '<div class="loader"></div>';
    getImage(imageId).then(image_data => {
        if (image_data instanceof Error) {
            displayError(container, imageId, image_data.message);
        } else {
            renderImageSVG(image_data, container);
        }
    });
}

function displayError(container: HTMLElement, imageId: number, error: string) {
    container.innerHTML = `<div class="error-message">${error}</div>`;
    let button = document.createElement('button');
    button.textContent = "Try again";
    button.onclick = () => loadImage(imageId, container);
    container.appendChild(button);
}

displayImages();
