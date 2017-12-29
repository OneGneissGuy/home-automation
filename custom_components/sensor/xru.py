"""
Sensor for Xbox Live account status using api.xboxrecord.us.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/sensor.xru_xbox_live/
"""
import logging
import voluptuous as vol
import urllib.request
from urllib.error import URLError, HTTPError
import json
import re

import homeassistant.helpers.config_validation as cv
from homeassistant.components.sensor import PLATFORM_SCHEMA
from homeassistant.const import (CONF_API_KEY, STATE_UNKNOWN)
from homeassistant.helpers.entity import Entity


_LOGGER = logging.getLogger(__name__)

CONF_GAMERTAGS = 'gamertags'

ICON = 'mdi:xbox'

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Required(CONF_GAMERTAGS): vol.All(cv.ensure_list, [cv.string])
})


# pylint: disable=unused-argument
def setup_platform(hass, config, add_devices, discovery_info=None):
    """Set up the Xbox platform."""
    devices = []
    _LOGGER.info('test')
    for gamertag in config.get(CONF_GAMERTAGS):
        new_device = XboxSensor(hass, gamertag)
        if new_device.success_init:
            devices.append(new_device)

    if devices:
        add_devices(devices)
    else:
        return False


class XboxSensor(Entity):
    """A class for the Xbox account."""

    def __init__(self, hass, gamertag):
        """Initialize the sensor."""
        self._hass = hass
        self._state = STATE_UNKNOWN
        self._presence = {}
        self._gamertag = gamertag
        tries = 0
        userDetails = []
        # get profile info
        
        
        while True:
            userDetails = self.fetch_user_details(self._gamertag)
            if ('status' in userDetails and userDetails['status'] == "success" or tries > 5):
                break
            tries = tries + 1
 
        if userDetails['status'] == "success":
            self.update()
            self.success_init = True
            for item in userDetails['userDetails']:
                if (item['id'] == "GameDisplayPicRaw"):
                    pic = item['value']
                    pic = re.sub("http://images-eds", "https://images-eds-ssl", pic)
                    self._picture = self._picture = pic
        else:
            _LOGGER.critical("Failed to setup {}, this gamertag will be missing".format(gamertag))
            self.success_init = False

    @property
    def name(self):
        """Return the name of the sensor."""
        return self._gamertag

    @property
    def state(self):
        """Return the state of the sensor."""
        return self._state

    @property
    def device_state_attributes(self):
        """Return the state attributes."""
        attributes = {}

        if (self._state == "Online"):
            for device in self._presence['userPresence'][0]['devices']:
                for title in device['titles']:
                    if (title['placement'] == "Full"):
                        attributes[
                            '{}'.format(title['name'])
                        ] = '{}'.format(device['type'])
        else:
            attributes[
                'Offline'
            ] = 'Offline'
                
        return attributes

    @property
    def entity_picture(self):
        """Avatar of the account."""
        return self._picture

    @property
    def icon(self):
        """Return the icon to use in the frontend."""
        return ICON

    def update(self):
        """Update state data from Xbox API."""
        presence = self.fetch_user_presence(self._gamertag)
        self._state = presence['userPresence'][0]['current_activity']['name']
        self._presence = presence

    def fetch_user_details(self, gamertag):
        """ Fetches user details """
        data = []
        _LOGGER.info("Fetching user details for {}".format(gamertag))
        url = 'https://api.xboxrecord.us/userdetails/gamertag/{}'.format(gamertag)
        req = urllib.request.Request(url)
        try:
            raw_json = urllib.request.urlopen(req).read()
        except urllib.error.HTTPError as e:
            _LOGGER.critical(e.code)
            _LOGGER.critical(e.read())
            return {"status":"failure"}
        
        data = json.loads(raw_json.decode('utf-8'))

        return data

    def fetch_user_presence(self, gamertag):
        """ Fetches user presence """
        data = []
        _LOGGER.info("Fetching user presence for {}".format(gamertag))
        url = 'https://api.xboxrecord.us/userpresence/gamertag/{}'.format(gamertag)
        req = urllib.request.Request(url)
        try:
            raw_json = urllib.request.urlopen(req).read()
        except urllib.error.HTTPError as e:
            _LOGGER.critical(e.code)
            _LOGGER.critical(e.read())
            return {"status":"failure"}
        
        data = json.loads(raw_json.decode('utf-8'))

        return data

    def fetch_user_details_wrapper(self, gamertag):
        userDetails = self.fetch_user_details(self._gamertag)
        
        if userDetails['status'] == "success":
            
            self.success_init = True
            for item in userDetails['userDetails']:
                if (item['id'] == "GameDisplayPicRaw"):
                    pic = item['value']
                    pic = re.sub("http://images-eds", "https://images-eds-ssl", pic)
                    self._picture = self._picture = pic

        return userDetails