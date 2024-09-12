Inspirowane https://www.youtube.com/watch?v=34jQRPssM5Q

1. **pip install fastapi**
2. **pip install uvicorn**
3. **pip install sqlalchemy**

**uvicorn images:app --reload**
**127.0.0.1:8000/docs**

### Przykładowy curl
(Swagger też je generuje)

curl -X 'GET'  'http://localhost:8000/tags' -H 'accept: application/json'

curl -X 'GET'  'http://localhost:8000/images' -H 'accept: application/json'

curl -X 'POST' \
  'http://127.0.0.1:8000/images' \
  -H 'accept: application/json' \
  -H 'Content-Type: application/json' \
  -d '{
  "title": "string",
  "tags": [
    "jakis_tag"
  ]
}'

curl -X 'GET'  'http://localhost:8000/images/jakis_tag' -H 'accept: application/json'