from datetime import datetime, timedelta
from django.core.urlresolvers import reverse

from django.template.defaulttags import register
from django.utils import timezone

from main.helpers import build_navigation_entries
from main.models import WeatherData


@register.inclusion_tag('nav/navigation.html')
def render_nav_year(year):
    min_date = WeatherData.objects.order_by('recorded_at')[0].recorded_at
    entries = list(build_navigation_entries(
        start_date=min_date,
        end_date=timezone.now(),
        offset=timedelta(366),
    ))
    return {
        'entries': entries,
        'format': 'Y',
        'current': year,
        'url_name': 'year',
    }

@register.inclusion_tag('nav/navigation.html')
def render_nav_month(year, month):
    entries = list(build_navigation_entries(
        start_date=datetime(year, 1, 1),
        end_date=datetime(year + 1, 1, 1),
        offset=timedelta(days=31),
    ))

    return {
        'entries': entries,
        'format': 'm',
        'current': month,
        'year': year,
        'url_name': 'month',
    }

@register.inclusion_tag('nav/navigation.html')
def render_nav_day(year, month, day):
    entries = list(build_navigation_entries(
        start_date=datetime(year, month, 1),
        end_date=datetime(year, month + 1, 1) - timedelta(days=1),
        offset=timedelta(days=1)
    ))

    return {
        'entries': entries,
        'format': 'd',
        'current': day,
        'year': year,
        'month': month,
        'url_name': 'day',
    }


@register.simple_tag()
def get_nav_url(url_name, year, month, label):
    """ given a lambda and a value, call it
    """
    if not year:
        return reverse(url_name, args=(label, ))
    if not month:
        return reverse(url_name, args=('%04d' % year, label, ))
    return reverse(url_name, args=('%04d' % year, '%02d' % month, label, ))
