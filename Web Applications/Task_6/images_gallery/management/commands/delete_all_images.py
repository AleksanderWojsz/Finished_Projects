from django.core.management.base import BaseCommand
from images_gallery.models import Image


class Command(BaseCommand):

    def handle(self, *args, **options):
        Image.objects.all().delete()

