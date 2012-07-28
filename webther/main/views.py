import calendar

from django.http import HttpResponse
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
    def post(self, request):
        print "=" * 100
        print request.POST
