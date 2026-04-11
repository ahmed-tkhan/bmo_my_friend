"""
Simple sky-clearness checker prototype using OpenWeatherMap Current Weather API.
If cloudiness < threshold, it will notify the host (print/send command).

Requires: requests, pyserial (optional if you want to send serial commands directly)

Configure with environment variable `OWM_API_KEY` or pass as --key.
"""
import os
import sys
import time
import argparse
import requests
from serial_client import SerialClient

API_URL = 'https://api.openweathermap.org/data/2.5/weather'

def check_cloudiness(lat, lon, api_key):
    params = {'lat': lat, 'lon': lon, 'appid': api_key}
    r = requests.get(API_URL, params=params, timeout=10)
    r.raise_for_status()
    j = r.json()
    # OpenWeatherMap: 'clouds': {'all': percent}
    clouds = j.get('clouds', {}).get('all', None)
    return clouds

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--lat', type=float, required=True)
    parser.add_argument('--lon', type=float, required=True)
    parser.add_argument('--key', help='OpenWeatherMap API key (or set OWM_API_KEY)')
    parser.add_argument('--port', help='Serial port to device (optional)')
    parser.add_argument('--threshold', type=int, default=20, help='cloud percent threshold to consider "clear"')
    args = parser.parse_args()

    api_key = args.key or os.getenv('OWM_API_KEY')
    if not api_key:
        print('OpenWeatherMap API key required (set --key or OWM_API_KEY)')
        sys.exit(1)

    clouds = check_cloudiness(args.lat, args.lon, api_key)
    print('Current cloudiness:', clouds)
    if clouds is not None and clouds <= args.threshold:
        print('Skies appear clear (<= %d%% clouds).' % args.threshold)
        if args.port:
            sc = SerialClient(args.port)
            # Example action: set a visual 'clear' face index 0 (adjust to your index)
            sc.send('EMOTION.SETIDX 0', wait_response=False)
            sc.close()
        else:
            print('No serial port given — skipping device update.')
    else:
        print('Skies not clear (clouds > threshold).')

if __name__ == '__main__':
    main()
