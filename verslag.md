# Verslag Computer Graphics Project
Groep 21 | Artuur Heidbuchel | Merlijn Van Suetendael

## Inhoud
- De Beziercurve
- Animatie
- Modellen/textures
- Visualisatie
- Camera
- Belichting
- Convelutie
- Post-processing
- Chroma-keying
- interactie
- extras


## De Beziercurve
Voor het circuit gebruiken we Bezier-curves, zoals de opdracht verwacht. Het circuit bestaat uit een reeks segmenten die samen de vorm van het circuit bepalen. Per segment worden vier punten gebruikt: een startpunt, twee controlepunten en een eindpunt. De positie op de curve op tijdstip t wordt berekend met de standaard kubische Bezier-formule. Om de segmenten vloeiend aan elkaar te laten aansluiten, berekenen we de tangenten via Catmull-Rom-spanning: de controlepunten worden afgeleid uit de buurpunten met een spannningsfactor van 0,35, zodat de richting in elk knooppunt continu is. Naast de positie berekenen we ook de eerste en tweede afgeleide van de curve. De eerste afgeleide geeft de rijrichting van de auto en dient om de rotatiematrix op te stellen. De tweede afgeleide wordt gecombineerd met de eerste om de kromlijn (curvature) te berekenen, wat vervolgens gebruikt wordt om te bepalen hoe het stuur en de wielen moeten draaien om de draai van de auto logisch te volgen.

## Animatie
De auto beweegt over het circuit door een globale parameter carT elke frame te verhogen met carSpeed * deltaTime, waarna die waarde wordt omgezet naar een segmentindex en een lokale t binnen dat segment. Zo wordt elke frame de positie van de auto opgehaald via sampleCircuit() en de rijrichting via sampleCircuitAfgeleide(). Uit die tangent en de wereldomhoog-vector wordt een rotatiematrix opgebouwd met drie assen — rechts, omhoog en vooruit — zodat de auto altijd in de rijrichting georiënteerd staat. Voor het stuur wordt via sampleCurvature() de kromming van de curve berekend, die geschaald wordt naar een stuurhoek tussen -180° en +180°. Die hoek wordt doorgegeven aan DrawCar(), die de wielen en het stuur draait op basis van die hoek. De wielrotatie zelf wordt bijgehouden via distanceTraveled, dat elke frame een eenheid omhoog gaat en ook meegegeven wordt aan DrawCar(). Zo roteren de wielen dus op een logische snelheid.

## Modellen/textures
Voor de textures en models hebben we online models gedownload en ingeladen in het project. Dat inladen gebeurt via de stbi_image library. Het circuit is een model, geen aaneenschakeling van "puzzelstukken".

## Visualisatie
Voor visualisatie hebben we een zwarte lijn die de beziercurve aangeeft. Deze staat los van het circuit, aangezien het circuit een standaar model is.

## Camera
De camera kent vier modi: follow-cam mode, first-person cam mode, free cam mode en een cinematic cam. In de volg en cinematic camera-modi wordt de camerapositie elke frame berekend als een vaste offset achter en boven de auto, op basis van de tangent van de curve, waarna SetLookAt() een view-matrix opstelt via glm::lookAt(). De first-person camera werkt hetzelfde maar zet de offset anders zodat de camera in de auto zit. De vrije camera werkt met Euler-hoeken: muisbewegingen passen de yaw en pitch aan, waarna updateCameraVectors() de front-, right- en up-vector herberekent, met een max pitch van ±89° om te voorkomen dat je kunt blijven draaien.

## Belichting


## Convolutie


## Post-Processing


## Chroma-keying 
Voor de chroma-keying wordt er een quad over de scene gerenderd, deze quad heeft een texture die grotendeels groen is. De quad heeft een shader die elke pixel die groen is discard, waardoor de scene erachter tevoorschijn komt.

## Interactie
In de rechterbovenhoek van het scherm wordt er een hotkey legende getoond, hier kunnen de verschillende vormen van interactie en filters getoggled. In de first/third person cam kan de autosnelheid worden aangepast. Door op Z/W, S, SHIFT en SPACE te drukken wordt de auto versneld, vertraagd, in achteruit gezet en gestopt. Daarnaast is er ook nog interactie in de vorm van picking. Deze picking is niet toegepast op een manier die in de opgave stond. De picking is een soort van raypicking, er wordt een "ray" uitgestuurd naar een specifieke richting en als die richting overeenkomt met de richting waar het object zich bevindt, wordt de functie uitgevoerd. Het object is hier de stapel van banden aan de ingang van de pitstop. Als daar op geklikt wordt, dan wordt er een soort pitstop mode toegepast. Die pitstopmode verwisselt de curve waar de auto op rijdt met de curve die langs de pitstop gaat.

## Extras

- Camera shake:

