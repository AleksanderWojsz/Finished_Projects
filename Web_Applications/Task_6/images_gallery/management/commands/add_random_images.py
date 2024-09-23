import random as rand
from django.core.management.base import BaseCommand
from images_gallery.models import Image, Rectangle, Tag
from django.contrib.auth.models import User
from random import randrange
from datetime import timedelta, timezone
from datetime import datetime
from django.utils import timezone

# https://stackoverflow.com/a/553448/22553511
def random_date(start, end):
    delta = end - start
    int_delta = (delta.days * 24 * 60 * 60) + delta.seconds
    random_second = randrange(int_delta)
    return start + timedelta(seconds=random_second)
d1 = datetime.strptime('1/1/2008 1:30 PM', '%m/%d/%Y %I:%M %p')
d2 = datetime.now()

class Command(BaseCommand):

    def add_arguments(self, parser):
        parser.add_argument('liczba', type=int)

    def handle(self, *args, **options):
        liczba = options['liczba']

        for i in range(liczba):
            image = Image.objects.create(
                title=f"Obrazek_{i}",
                width=rand.randint(50, 500),
                height=rand.randint(50, 500),
                description="Opis losowego obrazka",
                artist=User.objects.filter(is_superuser=True).first(),
                publication_date=timezone.make_aware(random_date(d1, d2)), # https://stackoverflow.com/questions/18622007/runtimewarning-datetimefield-received-a-naive-datetime
            )

            # dodawanie losowych tagów
            tags = list(Tag.objects.all())
            selected_tags = rand.sample(tags, k=rand.randint(1, min(len(tags), 5)))
            for tag in selected_tags:
                image.tags.add(tag)

            # Dodanie jednego konkretnego tagu
            # tag, created = Tag.objects.get_or_create(name='ładny')
            # image.tags.add(tag)

            # tworzenie prostokątów
            for j in range(10):
                 Rectangle.objects.create(
                     image=image,
                     x=rand.randint(0, image.width - 1),
                     y=rand.randint(0, image.height - 1),
                     width=rand.randint(10, image.width // 2),
                     height=rand.randint(10, image.height // 2),
                     color=rand.choice(['red', 'green', 'blue', 'black', 'yellow']),
                 )

