import requests


bot_token = '6246175382:AAFU9wnsIzrJ8nhk3j3AJIdbv8IVg2Rrh3s'
webhook_url = "http://192.168.1.11:5000/telegram_bot"

response = requests.post(
    f"https://api.telegram.org/bot{bot_token}/setWebhook", json={"url": webhook_url}
)

print("message from webhook request", response)