"""
Simple TTS notifier that speaks a message using `pyttsx3`.
Plays to the host default audio device (attach headphones as needed).

Usage: python notify_headphones.py "Skies are clear in Algonquin"
Requires: pyttsx3
"""
import sys
try:
    import pyttsx3
except Exception as e:
    print('pyttsx3 not installed. Install with: pip install pyttsx3')
    raise

def speak(text):
    engine = pyttsx3.init()
    engine.say(text)
    engine.runAndWait()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: python notify_headphones.py "message"')
        sys.exit(1)
    speak(' '.join(sys.argv[1:]))
