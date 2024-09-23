from django import forms

class RectangleForm(forms.Form):
    x = forms.IntegerField(label='X', min_value=0)
    y = forms.IntegerField(label='Y', min_value=0)
    width = forms.IntegerField(label='Szerokość', min_value=1)
    height = forms.IntegerField(label='Wysokość', min_value=1)
    color = forms.CharField(label='Kolor', max_length=100)
