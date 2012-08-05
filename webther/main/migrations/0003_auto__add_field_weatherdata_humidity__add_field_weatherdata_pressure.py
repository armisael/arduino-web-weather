# -*- coding: utf-8 -*-
import datetime
from south.db import db
from south.v2 import SchemaMigration
from django.db import models


class Migration(SchemaMigration):

    def forwards(self, orm):
        # Adding field 'WeatherData.humidity'
        db.add_column('main_weatherdata', 'humidity',
                      self.gf('django.db.models.fields.FloatField')(null=True),
                      keep_default=False)

        # Adding field 'WeatherData.pressure'
        db.add_column('main_weatherdata', 'pressure',
                      self.gf('django.db.models.fields.FloatField')(null=True),
                      keep_default=False)


    def backwards(self, orm):
        # Deleting field 'WeatherData.humidity'
        db.delete_column('main_weatherdata', 'humidity')

        # Deleting field 'WeatherData.pressure'
        db.delete_column('main_weatherdata', 'pressure')


    models = {
        'main.weatherdata': {
            'Meta': {'ordering': "('-modified', '-created')", 'object_name': 'WeatherData'},
            'created': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now', 'blank': 'True'}),
            'humidity': ('django.db.models.fields.FloatField', [], {'null': 'True'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'modified': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now', 'blank': 'True'}),
            'pressure': ('django.db.models.fields.FloatField', [], {'null': 'True'}),
            'recorded_at': ('django.db.models.fields.DateTimeField', [], {'unique': 'True'}),
            'temperature': ('django.db.models.fields.FloatField', [], {'null': 'True'})
        }
    }

    complete_apps = ['main']