from django.contrib import admin
from .models import Image, Tag

class ImageAdmin(admin.ModelAdmin):
    fields = ['title', 'width', 'height', 'artist', 'tags', 'description']

admin.site.register(Image, ImageAdmin)
admin.site.register(Tag)
