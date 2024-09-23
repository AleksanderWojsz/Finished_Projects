from django.urls import path
from .views import images_list, image_detail, rectangle_delete, my_images, image_detail_editable

urlpatterns = [
    path('', images_list, name='images_list'),
    path('image_edit/<int:pk>/', image_detail_editable, name='image_detail_editable'),
    path('image/<int:pk>/', image_detail, name='image_detail'),
    path('image/<int:pk>/rectangle/<int:rectangle_pk>/delete/', rectangle_delete, name='rectangle_delete'),
    path('my-images/', my_images, name='my_images'),
]

