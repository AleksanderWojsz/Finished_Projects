from django.shortcuts import render, get_object_or_404, redirect
from .forms import RectangleForm
from .models import Image, Rectangle

def images_list(request):
    images = Image.objects.all()
    return render(request, 'images_gallery/all_images_list.html', {'images': images})

def image_detail(request, pk):
    image = get_object_or_404(Image, pk=pk)
    return render(request, 'images_gallery/image_detail.html', {'image': image, 'form': RectangleForm()})


def image_detail_editable(request, pk):
    image = get_object_or_404(Image, pk=pk)
    if request.method == 'POST':
        form = RectangleForm(request.POST)

        if form.is_valid():
            Rectangle.objects.create(
                image=image,
                x=form.cleaned_data['x'],
                y=form.cleaned_data['y'],
                width=form.cleaned_data['width'],
                height=form.cleaned_data['height'],
                color=form.cleaned_data['color']
            )
            return redirect('image_detail_editable', pk=image.pk)

    else:
        form = RectangleForm()
    return render(request, 'images_gallery/image_detail_editable.html', {'image': image, 'form': form})



def rectangle_delete(request, pk, rectangle_pk):
    image = get_object_or_404(Image, pk=pk)
    rectangle = get_object_or_404(Rectangle, pk=rectangle_pk, image=image)
    if request.method == 'POST':
        rectangle.delete()
        return redirect('image_detail_editable', pk=image.pk)
    return render(request, 'images_gallery/rectangle_confirm_delete.html', {'rectangle': rectangle})


def my_images(request):
    if request.user.is_superuser:
        images = Image.objects.all()
    else:
        images = Image.objects.filter(artist=request.user)
    return render(request, 'images_gallery/my_images_list.html', {'images': images})
