from django.conf.urls import patterns, url
from main.views import WeatherDataListView, ArduinoTimestamp, ArduinoPost


urlpatterns = patterns('',
    url(r'^$', WeatherDataListView.as_view()),
    url(r'^arduino-timestamp/$', ArduinoTimestamp.as_view()),
    url(r'^arduino-post/$', ArduinoPost.as_view()),
)
