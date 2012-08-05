import time
import calendar
from datetime import datetime, timedelta

from django.core.urlresolvers import reverse
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django.views.generic.base import TemplateView, RedirectView
from django.views.generic import View
from django.utils import timezone
from main.helpers import float_or_none

from main.models import WeatherData


class TodayView(RedirectView):
    def get_redirect_url(self):
        today = datetime.today()
        return reverse('day', args=('%04d' % today.year,
                                    '%02d' % today.month,
                                    '%02d' % today.day
            )
        )


class YearView(TemplateView):
    template_name = 'main/year.html'

    def get_context_data(self, **kwargs):
        return {
            'year': int(kwargs.get('year')),
        }


class MonthView(TemplateView):
    template_name = 'main/month.html'

    def get_context_data(self, **kwargs):
        return {
            'year': int(kwargs.get('year')),
            'month': int(kwargs.get('month')),
        }


class DayView(TemplateView):
    template_name = 'main/day.html'

    def get_context_data(self, **kwargs):
        day = datetime(int(kwargs.get('year')),
                       int(kwargs.get('month')),
                       int(kwargs.get('day')),
                       0, 0, 0, tzinfo=timezone.get_default_timezone())
        to_date = day + timedelta(days=1)
        return {
            'year': day.year,
            'month': day.month,
            'day': day.day,
            'data': WeatherData.objects.filter(recorded_at__gte=day,
                                               recorded_at__lt=to_date)
        }


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
