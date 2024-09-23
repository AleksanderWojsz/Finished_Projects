from bs4 import BeautifulSoup
import requests
from googlesearch import search

def prepare_descriptions():
    title = wiki_soup.find("h1", id="firstHeading").text
    description = wiki_soup.find("div", class_="mw-content-ltr mw-parser-output").find_all("p")[1].text
    table_headers = " | ".join(rows[1].text.strip().replace("\n", "| ").split("| ")[:3])  # lączy trzy pierwsze elementy oddzielając je |

    table_page.write("## " + title + "\n\n" + description + "\n\n **" + rows[0].text.strip() + "** \n\n")
    table_page.write("| Position |" + table_headers + " | pictures & links |\n")
    table_page.write("| --- | --- | --- | --- | --- |\n")


def create_subpage(country_name, city_name):
    subpage_filename = f"subpage{position}.md"
    table_page.write(f" | [browse]({subpage_filename}) |")

    monument_name = cells[0].text.strip()

    with open(subpage_filename, "w", encoding="utf-8") as subpage_file:
        subpage_file.write("### Additional info: \n\n")
        for url in search(monument_name + " -site:https://en.wikipedia.org", stop=3):
            subpage_file.write(f"- [{url}]({url})\n")
        subpage_file.write("\n\n --- \n\n **Pictures**: \n\n")

        headers = {
            # https://stackoverflow.com/questions/72805266/python-web-scraping-code-error-http-error-406-not-acceptable
            'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36',
        }
        shutter_html_text = requests.get("https://www.shutterstock.com/pl/search/" + monument_name + " " + country_name + " " + city_name, headers=headers).text
        shutter_soup = BeautifulSoup(shutter_html_text, "lxml")
        images = shutter_soup.find("div", class_="mui-1nl4cpc-gridContainer-root")
        for image in images.find_all("div", role="img", limit=20):
            image_url = image.find("img").get("src")
            subpage_file.write(f"![Obrazek]({image_url})\n\n")


def create_table_row(position):
    table_page.write(f"| {position}")
    for i in range(3):
        table_page.write(" | ")
        if i == 1:  # czy kolumna z linkiem do miasta
            city_links = cells[i].find_all("a")

            country_flag_url = "https:" + city_links[0].find("img").get("src")
            table_page.write(f"![Flaga]({country_flag_url}) ")

            country_name = city_links[0].get("title")
            country_url = "https://en.wikipedia.org" + city_links[0].get("href")
            table_page.write(f"[{country_name}]({country_url}), ")

            city_name = city_links[1].text.strip()
            city_url = "https://en.wikipedia.org" + city_links[1].get("href")
            table_page.write(f"[{city_name}]({city_url})")
        else:
            table_page.write(cells[i].text.strip())
    return country_name, city_name

wiki_html_text = requests.get("https://en.wikipedia.org/wiki/List_of_most_visited_palaces_and_monuments").text
wiki_soup = BeautifulSoup(wiki_html_text, "lxml")

full_table = wiki_soup.find("table", class_="wikitable sortable")
rows = full_table.find_all("tr")

with open("table_page.md", "w", encoding="utf-8") as table_page:
    prepare_descriptions()
    position = 1

    for row in rows[2:]:  # pomija nagłówek
        cells = row.find_all("td")

        country_name, city_name = create_table_row(position)
        create_subpage(country_name, city_name)

        position += 1
        table_page.write("\n")

with open("index.md", "w", encoding="utf-8") as main_page:
    main_page.write("# Fan zabytków \n\n")
    main_page.write("\n\n Hej, jeśli tak jak ja, jesteś fanem zabytków i szukasz następnego celu podróży, to polecam Ci tę strone: \n\n")
    main_page.write(f"[Most visited monuments (with pictures!)](table_page.md) \n\n")
    main_page.write("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.")