from django.shortcuts import render, get_object_or_404, redirect
from .forms import RectangleForm
from .models import Image, Rectangle, Tag
from django.core.paginator import Paginator

def all_images_list(request):
    sort_by = request.GET.get('sort_by', 'desc') # jak parametr nie istnieje to jest 'desc'

    tag_list = request.GET.getlist('tags')

    # wersja gdzie wszystkie tagi muszą pasować
    images = Image.objects.all()
    for tag in tag_list:
        images = images.filter(tags=tag)

    # wersja gdzie co najmniej jeden tag musi pasować
    # filtered_images = Image.objects.filter(tags__name__in=tag_list).distinct() # distinct bo inaczej będziemy brali te sami obrazki kilka razy

    if sort_by == 'asc':
        images = images.order_by('publication_date')
    elif sort_by == 'desc':
        images = images.order_by('-publication_date')

    paginator = Paginator(images, 12)
    page_number = request.GET.get('page')
    images_on_page = paginator.get_page(page_number)

    return render(request, 'images_gallery/all_images_list.html', {'images': images_on_page, 'all_tags': Tag.objects.all()})

def image_detail(request, pk):
    image = get_object_or_404(Image, pk=pk)
    return render(request, 'images_gallery/image_detail.html', {'image': image})


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
