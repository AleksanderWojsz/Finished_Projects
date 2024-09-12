from django.conf import settings
from django.db import models
from django.utils import timezone

class Tag(models.Model):
    name = models.CharField(max_length=100, primary_key=True)
    def __str__(self):
        return self.name

class Image(models.Model):
    title = models.CharField(max_length=255)
    svg_data = models.TextField()
    width = models.IntegerField(default=100)
    height = models.IntegerField(default=100)
    artist = models.ForeignKey(settings.AUTH_USER_MODEL, on_delete=models.CASCADE, related_name='assigned_images', null=True, blank=True)
    description = models.TextField(blank=True, null=True)
    publication_date = models.DateTimeField(default=timezone.now)
    tags = models.ManyToManyField(Tag, related_name='images')

    def __str__(self):
        return self.title

class Rectangle(models.Model):
    image = models.ForeignKey(Image, related_name='rectangles', on_delete=models.CASCADE)
    x = models.IntegerField()
    y = models.IntegerField()
    width = models.IntegerField()
    height = models.IntegerField()
    color = models.CharField(max_length=100)

    def __str__(self):
        return f"Rectangle for {self.image.title} at ({self.x}, {self.y})"
