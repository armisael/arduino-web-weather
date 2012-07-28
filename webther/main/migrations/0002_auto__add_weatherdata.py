# -*- coding: utf-8 -*-
import datetime
from south.db import db
from south.v2 import SchemaMigration
from django.db import models


class Migration(SchemaMigration):

    def forwards(self, orm):
        # Adding model 'WeatherData'
        db.create_table('main_weatherdata', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('created', self.gf('django.db.models.fields.DateTimeField')(default=datetime.datetime.now, blank=True)),
            ('modified', self.gf('django.db.models.fields.DateTimeField')(default=datetime.datetime.now, blank=True)),
            ('recorded_at', self.gf('django.db.models.fields.DateTimeField')(unique=True)),
            ('temperature', self.gf('django.db.models.fields.FloatField')(null=True)),
        ))
        db.send_create_signal('main', ['WeatherData'])


    def backwards(self, orm):
        # Deleting model 'WeatherData'
        db.delete_table('main_weatherdata')


    models = {
        'main.weatherdata': {
            'Meta': {'ordering': "('-modified', '-created')", 'object_name': 'WeatherData'},
            'created': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now', 'blank': 'True'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'modified': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now', 'blank': 'True'}),
            'recorded_at': ('django.db.models.fields.DateTimeField', [], {'unique': 'True'}),
            'temperature': ('django.db.models.fields.FloatField', [], {'null': 'True'})
        }
    }

    complete_apps = ['main']