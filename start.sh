#!/bin/bash
gunicorn blockchain_explorer:app --bind 0.0.0.0:$PORT
