from django.db import models
from django_extensions.db.models import TimeStampedModel


class WeatherData(TimeStampedModel):
    """ weather data received by the arduino
    """
    recorded_at = models.DateTimeField(unique=True)
    temperature = models.FloatField(null=True)
    humidity = models.FloatField(null=True)
    pressure = models.FloatField(null=True)
