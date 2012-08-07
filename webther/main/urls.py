from django.conf.urls import patterns, url
from main.views import TodayView, YearView, MonthView, DayView, \
    ArduinoTimestamp, ArduinoPost


urlpatterns = patterns('',
    url(r'^$', TodayView.as_view()),
    url(r'^(?P<year>\d{4})/$', YearView.as_view(), name='year'),
    url(r'^(?P<year>\d{4})/(?P<month>\d{2})/$', MonthView.as_view(), name='month'),
    url(r'^(?P<year>\d{4})/(?P<month>\d{2})/(?P<day>\d{2})/$', DayView.as_view(), name='day'),
    url(r'^arduino-timestamp/$', ArduinoTimestamp.as_view()),
    url(r'^arduino-post/$', ArduinoPost.as_view()),
)
