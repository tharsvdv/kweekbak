# kweekbak
Smart Greenhouse IoT

We hebben een zeer grote serre die onderverdeeld is in meerdere kweekbakken met verschillende
soorten kruiden. Omdat de kweekbakken geregeld van kruid veranderen wil de eigenaar graag een
modulair systeem om deze te automatiseren en monitoren.
Elke kweekbak regelt afzonderlijk de ideale kweekomgeving voor het gekozen kruid. Zo zal een
kweekbak voor basilicum andere temperatuur, vochtigheid en belichtingsperiodes hebben dan een
kweekbak voor dille.
Elke kweekbak zal dus sensoren nodig hebben die de grondvochtigheid en temperatuur monitoren.
Indien een vooraf bepaalde grenswaarde overschreden wordt zal er een bepaalde
automatisatie/actie ondernomen worden. (te warm -> vensters open, te droog -> water pompen,
voedingstoffen toevoegen?, ...) De belichting gebeurd ook via een automatisch programma.
De data van deze sensoren en de bijhorende acties worden opgeslagen in een database en kunnen
worden weergegeven met grafana.
Om te voorkomen dat de kweker zijn kweekbakken opnieuw moet programmeren bij elke
kruidwisseling zal hij door middel van een RFID tag de bak automatisch kunnen instellen op de
gewenste kruiden. Deze instellingen kunnen worden weergegeven op een LCD.

als je het project na wilt maken kan je naar schema en pcbdesign bestanden gaan kijken in deze repository. veel van die bestanden staan ook in het documentatie bestand
het pcb design is niet getest geweest maar zou wel moeten werken.

eerst zorg je ervoor dat je alle nodige hardware hebt:
ESP32, raspberry pi, moisture sensor, 5V relais, 5V waterpompje, LDR, ledstrip, DHT11, 5V servomotor, bodemtemperatuursensor DS18B20, led(deze dient voor te simuleren dat een warmtemat aangaat), ultrasone sensor HC-SR04. een extra 5V voeding is ook nodig voor het pompje. ook een doorzichtige bak en een plantenpotje voor de plant.

test eerst alle hardware door het in elkaar te steken zoals in het schema bestand. je moet alleen nog de 5V pomp vebinden met de relais en de extra 5V bron. je steekt de - van de pomp aan de - van de 5V bron en de + in de middelste com poort van de relais. de + van de 5V bron verbind je met de NO (normally open) poort van de relais.

dan kopieer je de code van het .ino bestand en plak je die in je arduino IDE applicatie op je pc. die verbind je met je esp32. vergeet niet al de libraries die bij de code horen allemaal te downloaden. verander in het bestand je wifi ssid en password zodat de esp kan verbinden. verander ook de mqtt server naar de naam van jouw raspberry pi

verbind je raspberry pi met je internet via wifi of ethernet kabel en maak een python bestand dat je vind in de repository of het documentatie bestand. maak ook in je influxdb een database aan genaamd kweekbak en een user met wachtwoord. vergeet die niet aan te passen in de python code

nu mag je de code uploaden van arduino naar je esp32 en op je raspberry het python bestand runnen. als alles juist gedaan is zou je op je raspberry te zien krijgen dat data naar een influxdb succesvol gestuurd worden. voor te testen of het werkt kan je op influxdb op je raspberry een sql querry doen in je database voor alles te zien

nu kan je op grafana gaan door op je browser het ip adres van je raspberry te zetten. voeg hier je kweekbak database toe als data source. dan ga je naar dashboard een maak een nieuw visualitatie met dezelfde sql querry als op je influxdb en dan kan je je data zien in een grafiek.

nu mag je van je python code een service maken zodat die werkt vanaf je de raspberry aanzet.

als dat klaar is kan je proberen alles te solderen aan een prototype board of op het pcb bord maar die is nog nooit getest dus het kan zijn dat die niet werkt. doe zeker genoeg tests voor te zien of het volledig werkt

dan mag je een behuizing maken. wat ik heb gedaan is met dubbelijdige tape de 3 prototype boards dat ik heb gemaakt plakken aan de binnenkant en voor watertank heb ik een plastieke fles open gesneden. ik heb ook een aantal gaten voor kabels gemaakt voor de rfid, LCD, sonde van de waterpomp, relais, ultrasone sensor en de kabel voor de esp met de pc te verbinden voor te testen