import time
import calendar
from datetime import datetime

from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.generic.list import ListView
from django.views.generic import View
from django.utils import timezone

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

        recorded_at = datetime(*time.gmtime(request.POST['timezone'])[:6],
                               tzinfo=timezone.utc),

        WeatherData.objects.create(
            recorded_at=recorded_at,
            temperature=request.POST.get('temperature'),
            humidity=request.POST.get('humidity'),
            pressure=request.POST.get('pressure'),
        )
        return HttpResponse(content='', status=200)
