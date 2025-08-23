import pandas as pd
import json
from fastapi import FastAPI, HTTPException

app = FastAPI(debug=True)

def get_value(drugbank_id):

    try:

        df = pd.read_csv('files/pathways.csv')
        result = []
        for _, row in df.iterrows(): # Przechodzę po każdym wierszu i parsuję jsona
            drugs_for_pathway = json.loads(row['drugs'])['drug']

            if not isinstance(drugs_for_pathway, list):
                drugs_for_pathway = [drugs_for_pathway]

            for drug in drugs_for_pathway:
                result.append({'smpdb-id': row['smpdb-id'], 'drugbank-id': drug['drugbank-id'], 'name': drug['name']})

        main_df = pd.read_csv('files/main.csv')
        result = main_df.merge(pd.DataFrame(result), on='drugbank-id', how='left')
        result = result[['drugbank-id', 'smpdb-id']]
        result = result.groupby('drugbank-id').count()

        return result.loc[drugbank_id, 'smpdb-id'].item() # `loc` daje typ z numpy, więc używam item(), żeby mieć wartość z native pythona

    except KeyError:
        raise HTTPException(status_code=404, detail=f"ID '{drugbank_id}' nie istnieje")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Błąd: {str(e)}")


@app.post("/number_of_pathways/{drugbank_id}")
async def number_of_pathways(drugbank_id: str):
    return {f"Liczba szlaków dla {drugbank_id}": get_value(drugbank_id)}


# Instalacja fastapi   pip install "fastapi[standard]"
# Uruchomienie:        fastapi dev zadanie_15.py
# Swagger:             http://127.0.0.1:8000/docs
# Przykładowe ID       DB00001
