import sqlite3
from datetime import datetime

import requests
from flask import Flask, request

app = Flask(__name__)
# Define your bot token
bot_token = '6246175382:AAFU9wnsIzrJ8nhk3j3AJIdbv8IVg2Rrh3s'

# Define the base URL for the Telegram API
base_url = f'https://api.telegram.org/bot{bot_token}/'

# Define the chat ID of the recipient
chat_id = 5467462666  # Replace with the actual chat ID


@app.route('/')
def home_page():
    return ("""
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Home Page</title>
    <!-- Add any CSS or meta tags for styles and SEO here -->
</head>
<body>
    <header>
        <h1>Welcome to Our Website</h1>
        <!-- Add navigation or header content here -->
    </header>

    <main>
        <section>
            <h2>About Us</h2>
            <p>This is a brief description of our website or organization.</p>
        </section>

        <section>
            <h2>Services</h2>
            <ul>
                <li>Service 1</li>
                <li>Service 2</li>
                <li>Service 3</li>
            </ul>
        </section>
    </main>

    <footer>
        <p>&copy; 2023 Your Website Name. All Rights Reserved.</p>
        <!-- Add footer content, links, or copyright information here -->
    </footer>
</body>
</html>

    """)


@app.route('/receive_data', methods=['GET'])
def receive_data():
    try:
        message = request.args.get('message')
        num = request.args.get('num')  # retrieving data from NodeMCU in JSON format
        send_message(message)
        if num == "abc":  # only add to database if the pill is taken
            current_datetime = datetime.now()  # get current date and time
            date_component = current_datetime.date()
            time_component = current_datetime.time()

            # Connect to the SQLite database
            conn = sqlite3.connect('pill_dispenser.db')
            cursor = conn.cursor()
            cursor.execute(
                '''
                CREATE TABLE IF NOT EXISTS timetable (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    date_column DATE,
                    time_column TIME
                );
                '''
            )
            # Insert data into the 'time_data' table
            cursor.execute("INSERT INTO timetable (date_column, time_column) VALUES (?, ?)", (date_component,
                                                                                              time_component.strftime(
                                                                                                  "%H:%M:%S")))
            conn.commit()
            conn.close()
            return 'Data received and stored in the database'
        print("the client sent the following data: ", message)
        return 'Message Sent Successfully'

    except Exception as e:
        return f'An error occurred: {str(e)}'


# @app.route('/send_message')
def send_message(message_text):
    # Compose the API request URL
    send_message_url = f'{base_url}sendMessage'

    # Prepare the message data
    message_data = {
        'chat_id': chat_id,
        'text': message_text
    }

    # Send the message
    response = requests.post(send_message_url, json=message_data)

    # Check the response
    if response.status_code == 200:
        print('Message sent successfully!')
    else:
        print('Failed to send message. Status code:', response.status_code)
        print('Response content:', response.text)

# @app.route('/telegram_bot')
# def telegram_bot():
#     print("absf")


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)  # Run the Flask app
