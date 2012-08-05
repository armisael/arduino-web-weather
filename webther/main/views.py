import time
import calendar
from datetime import datetime

from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.generic.list import ListView
from django.views.generic import View
from django.utils import timezone
from main.helpers import float_or_none

from main.models import WeatherData


class WeatherDataListView(ListView):
    model = WeatherData


class ArduinoTimestamp(View):
    def get(self, request):
        t_now = calendar.timegm(timezone.now().timetuple())
        return HttpResponse('{0}'.format(t_now))


class ArduinoPost(View):

    # TODO[sp] protect this view from outsite the local network
    @csrf_exempt
    def dispatch(self, request, *args, **kwargs):
        return super(ArduinoPost, self).dispatch(request, *args, **kwargs)

    def post(self, request):
        print "=" * 100
        print request.POST

        timetuple = time.gmtime(float(request.POST['timestamp']))[:6]
        recorded_at = datetime(*timetuple, tzinfo=timezone.utc)
        temperature = float_or_none(request.POST.get('temperature'))
        humidity = float_or_none(request.POST.get('humidity'))
        pressure = float_or_none(request.POST.get('pressure'))

        print recorded_at, temperature, humidity, pressure

        WeatherData.objects.create(
            recorded_at=recorded_at,
            temperature=temperature,
            humidity=humidity,
            pressure=pressure,
        )
        return HttpResponse(content='', status=200)
