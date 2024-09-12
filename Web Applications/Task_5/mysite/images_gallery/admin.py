from django.contrib import admin
from .models import Image

class ImageAdmin(admin.ModelAdmin):
    list_display = ('title', 'width', 'height', 'artist')
    fields = ['title', 'width', 'height', 'artist']

admin.site.register(Image, ImageAdmin)
