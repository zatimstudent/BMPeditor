# Dokumentace projektu BMP Image Editor

## 1. Úvod

BMP Image Editor je desktopová aplikace vytvořená pomocí frameworku Qt, která umožňuje uživatelům otevírat, prohlížet, upravovat a ukládat soubory ve formátu BMP. Aplikace nabízí základní funkce pro úpravu obrazu, jako je inverze barev, rotace a horizontální převrácení. Dále poskytuje detailní informace o struktuře BMP souboru včetně hlaviček, metadat a barevné palety.

## 2. Architektura projektu

Projekt je strukturován do několika klíčových tříd, které zajišťují oddělení logiky, uživatelského rozhraní a zpracování obrazu:

- **MainWindow**: Hlavní okno aplikace obsahující UI komponenty a hlavní logiku
- **Image**: Třída pro práci s BMP soubory včetně načítání, ukládání a interpretace dat
- **CustomImageWidget**: Specializovaný widget pro vykreslování obrázků s podporou zoomu
- **Filter** a jeho podtřídy: Hierarchie tříd implementující jednotlivé obrazové filtry

## 3. Klíčové funkce pro zpracování obrazu

### 3.1 Načítání BMP souborů

Načítání BMP souborů je implementováno v metodě `Image::loadFromFile()`. Tato metoda detailně zpracovává strukturu BMP souboru:

```cpp
bool Image::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Načtení file header
    QByteArray fileHeaderData = file.read(14);
    if (fileHeaderData.size() < 14 || fileHeaderData[0] != 'B' || fileHeaderData[1] != 'M') {
        file.close();
        return false;
    }

    // Parsování file header
    fileHeader.bfType[0] = fileHeaderData[0];
    fileHeader.bfType[1] = fileHeaderData[1];
    fileHeader.bfSize = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 2);
    fileHeader.bfReserved1 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 6);
    fileHeader.bfReserved2 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 8);
    fileHeader.bfOffBits = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 10);

    // Načtení info header
    QByteArray infoHeaderData = file.read(40);
    if (infoHeaderData.size() < 40) {
        file.close();
        return false;
    }

    // Parsování info header
    infoHeader.biSize = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 0);
    infoHeader.biWidth = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 4);
    infoHeader.biHeight = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 8);
    infoHeader.biPlanes = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 12);
    infoHeader.biBitCount = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 14);
    infoHeader.biCompression = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 16);
    infoHeader.biSizeImage = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 20);
    infoHeader.biXPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 24);
    infoHeader.biYPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 28);
    infoHeader.biClrUsed = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 32);
    infoHeader.biClrImportant = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 36);

    // Kontrola podporovaných formátů
    if (infoHeader.biCompression != 0 || 
        (infoHeader.biBitCount != 1 && infoHeader.biBitCount != 4 && 
         infoHeader.biBitCount != 8 && infoHeader.biBitCount != 24)) {
        file.close();
        return false;
    }

    // Nastavení základních parametrů
    imageWidth = infoHeader.biWidth;
    imageHeight = abs(infoHeader.biHeight);
    imageBitsPerPixel = infoHeader.biBitCount;


    // Načtení palety
    colorPalette.clear();
    if (imageBitsPerPixel <= 8) {
        int paletteSize = (infoHeader.biClrUsed > 0) ? infoHeader.biClrUsed : (1 << imageBitsPerPixel);
        QByteArray paletteData = file.read(paletteSize * 4);

        for (int i = 0; i < paletteSize && i*4 < paletteData.size(); i++) {
            int blue = static_cast<unsigned char>(paletteData[i*4]);
            int green = static_cast<unsigned char>(paletteData[i*4+1]);
            int red = static_cast<unsigned char>(paletteData[i*4+2]);
            colorPalette.append(qRgb(red, green, blue));
        }
    }

    // Čtení dat obrázku
    file.seek(fileHeader.bfOffBits);
    rawData = file.readAll();
    file.close();

    // Převedení raw dat do QImage
    renderFromRawData();
    sourceFilePath = filePath;
    modified = false;
    
    return true;
}
```

Metoda postupuje následovně:
1. Otevře soubor a ověří jeho validitu (zda začíná identifikátorem "BM")
2. Načte a zpracuje File Header (14 bajtů) a Info Header (40 bajtů)
3. Pro obrázky s barevnou paletou (1, 4 nebo 8 bitů na pixel) načte paletu
4. Načte obrazová data a uloží je jako raw data
5. Převede raw data do interní reprezentace QImage pomocí `renderFromRawData()`

### 3.2 Vykreslování obrazových dat

Převod surových BMP dat do formátu QImage je klíčovou funkcí pro zobrazení obrázku v aplikaci. Toto zajišťuje metoda `Image::renderFromRawData()`:

```cpp
void Image::renderFromRawData() {
    // Vytvoření prázdného obrázku
    qImage = QImage(imageWidth, imageHeight, QImage::Format_RGB32);
    int bytesPerRow = calculateRowSize();

    // Vykreslení podle bitové hloubky
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            QRgb pixelColor;

            // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
            int row = (infoHeader.biHeight > 0) ? imageHeight - 1 - y : y;
            int byteIndex = row * bytesPerRow;

            if (imageBitsPerPixel == 24) {
                // Zpracování 24-bitových obrázků (3 bajty na pixel: B, G, R)
                int index = byteIndex + x * 3;
                if (index + 2 < rawData.size()) {
                    int blue = static_cast<unsigned char>(rawData[index]);
                    int green = static_cast<unsigned char>(rawData[index + 1]);
                    int red = static_cast<unsigned char>(rawData[index + 2]);
                    pixelColor = qRgb(red, green, blue);
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            else if (imageBitsPerPixel == 8) {
                // Zpracování 8-bitových obrázků (1 bajt = index do palety)
                int index = byteIndex + x;
                if (index < rawData.size()) {
                    int colorIndex = static_cast<unsigned char>(rawData[index]);
                    pixelColor = colorPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            // ... podobně pro 4-bitové a 1-bitové obrázky ...

            qImage.setPixel(x, y, pixelColor);
        }
    }
}
```

Tato metoda:
1. Vytváří nový QImage s určenými rozměry
2. Iteruje přes všechny pixely a dekóduje je podle bitové hloubky:
   - Pro 24-bitové obrázky: přímo načítá RGB hodnoty z raw dat
   - Pro 8, 4 a 1-bitové obrázky: získává indexy do barevné palety a načítá odpovídající barvy
3. Zajišťuje správné pořadí výšky (BMP ukládá data odspodu nahoru)
4. Ošetřuje různé uspořádání bitů v bajtech pro 4 a 1-bitové obrázky

### 3.3 Implementace zoomu v CustomImageWidget

CustomImageWidget poskytuje sofistikovanou implementaci vykreslování obrázků s možností přiblížení (zoom). Klíčové prvky této funkcionality:

```cpp
void CustomImageWidget::setZoomFactor(double factor) {
    // Omezení faktoru zoomu na rozumné hodnoty
    zoomFactor = qBound(0.1, factor, 10.0);
    update(); // Překreslit s novým faktorem
}

void CustomImageWidget::paintEvent(QPaintEvent* event) {
    if (image.isNull()) return;

    QPainter painter(this);

    int scaledWidth = qRound(image.width() * zoomFactor);
    int scaledHeight = qRound(image.height() * zoomFactor);

    // Vytvoření nového obrázku se zvětšenými pixely
    QImage scaledImage(scaledWidth, scaledHeight, QImage::Format_RGB32);

    // Vyplnění obrázku zvětšenými pixely
    for (int y = 0; y < image.height(); y++) {
        int scaledY = qRound(y * zoomFactor);
        int pixelHeight = qRound((y + 1) * zoomFactor) - scaledY;

        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int scaledX = qRound(x * zoomFactor);
            int pixelWidth = qRound((x + 1) * zoomFactor) - scaledX;

            // Vyplnění bloku pixelů stejnou barvou
            for (int sy = 0; sy < pixelHeight; sy++) {
                for (int sx = 0; sx < pixelWidth; sx++) {
                    if (scaledX + sx < scaledWidth && scaledY + sy < scaledHeight) {
                        scaledImage.setPixel(scaledX + sx, scaledY + sy, pixel);
                    }
                }
            }
        }
    }

    // Vykreslení připraveného obrázku
    int x_offset = (width() - scaledWidth) / 2;
    int y_offset = (height() - scaledHeight) / 2;
    painter.drawImage(x_offset, y_offset, scaledImage);
}
```

#### Podrobnější popis zoomu:

1. **Inicializace a příprava**
   ```cpp
   int scaledWidth = qRound(image.width() * zoomFactor);
   int scaledHeight = qRound(image.height() * zoomFactor);
   QImage scaledImage(scaledWidth, scaledHeight, QImage::Format_RGB32);
   ```
   - Nejprve se vypočítají nové rozměry zvětšeného obrázku vynásobením původních rozměrů faktorem zoomu
   - Vytvoří se nový prázdný obrázek `scaledImage` s těmito zvětšenými rozměry


2. **Vnější smyčka - iterace přes řádky původního obrázku**
   ```cpp
   for (int y = 0; y < image.height(); y++) {
   ```
   - Procházíme každý řádek původního obrázku od 0 po jeho výšku


3. **Výpočet pozice a výšky pro každý řádek po aplikování zoomu**
   ```cpp
   int scaledY = qRound(y * zoomFactor);
   int pixelHeight = qRound((y + 1) * zoomFactor) - scaledY;
   ```
   - `scaledY` je Y-ová souřadnice v zvětšeném obrázku, která odpovídá začátku aktuálního řádku
   - `pixelHeight` určuje výšku zvětšeného pixelu:
      - Vypočítá se jako rozdíl mezi Y-pozicí následujícího řádku po zvětšení a aktuální Y-pozicí
      - Tím se zajistí, že pokryjeme přesně prostor mezi dvěma sousedními pixely po zvětšení


4. **Vnitřní smyčka - iterace přes sloupce původního obrázku**
   ```cpp
   for (int x = 0; x < image.width(); x++) {
   ```
   - Pro každý řádek procházíme všechny sloupce (pixely) zleva doprava


5. **Získání hodnoty pixelu a výpočet jeho zvětšené pozice**
   ```cpp
   QRgb pixel = image.pixel(x, y);
   int scaledX = qRound(x * zoomFactor);
   int pixelWidth = qRound((x + 1) * zoomFactor) - scaledX;
   ```
   - Načteme barvu aktuálního pixelu z původního obrázku
   - `scaledX` je X-ová souřadnice v zvětšeném obrázku, která odpovídá začátku aktuálního pixelu
   - `pixelWidth` určuje šířku zvětšeného pixelu pomocí stejné logiky jako u výšky


6. **Vnitřní smyčky pro vyplnění bloku zvětšeného pixelu**
   ```cpp
   for (int sy = 0; sy < pixelHeight; sy++) {
       for (int sx = 0; sx < pixelWidth; sx++) {
   ```
   - Tyto dva cykly iterují přes všechny body v obdélníku, který reprezentuje původní pixel po zvětšení
   - Výška obdélníku je `pixelHeight` a šířka je `pixelWidth`
   - `sy` a `sx` jsou relativní offsety uvnitř tohoto obdélníku


7. **Kontrola hranic a vykreslení pixelu**
   ```cpp
   if (scaledX + sx < scaledWidth && scaledY + sy < scaledHeight) {
       scaledImage.setPixel(scaledX + sx, scaledY + sy, pixel);
   }
   ```
   - Ověříme, že nevykreslujeme mimo hranice cílového obrázku
   - Pro každou pozici v rámci obdélníku nastavíme stejnou barvu jako má původní pixel
   - Absolutní pozice v zvětšeném obrázku se vypočítá jako `scaledX + sx` pro X a `scaledY + sy` pro Y


8. **Vykreslení výsledného obrázku a jeho centrování**
   ```cpp
   int x_offset = (width() - scaledWidth) / 2;
   int y_offset = (height() - scaledHeight) / 2;
   painter.drawImage(x_offset, y_offset, scaledImage);
   ```
   - Nakonec se vypočítají offsety pro centrování obrázku ve widgetu
   - Výsledný zvětšený obrázek se vykreslí na plátno widgetu s těmito offsety

#### Výhody této implementace zoomu:

1. **Pixelově přesné zvětšení**: Každý pixel je reprezentován přesným obdélníkem, což zachovává původní pixelovou strukturu obrázku bez rozmazání nebo jiných artefaktů, které by mohly vzniknout při použití standardních funkcí škálování.

2. **Přesné rozměry**: Díky výpočtu `pixelWidth` a `pixelHeight` pro každý pixel se zajistí, že i při neceločíselných hodnotách faktoru zoomu bude celková velikost obrázku korektní.

3. **Efektivní a kontrolované vykreslování**: Kontroly hranic zabraňují přístupu mimo alokovanou paměť obrázku.

4. **Podpora libovolného faktoru zoomu**: Algoritmus funguje korektně jak pro zvětšení, tak i pro zmenšení obrázku.


Třída také poskytuje jednoduchá rozhraní pro ovládání zoomu:
- `zoomIn()`: Zvětšuje faktor zoomu o 25%
- `zoomOut()`: Zmenšuje faktor zoomu o 20%
- `resetZoom()`: Obnovuje výchozí faktor zoomu (1.0)
- `wheelEvent()`: Přepisuje zpracování události kolečka myši pro intuitivní ovládání zoomu

## 4. Ukládání BMP souborů

Ukládání obrázků je implementováno v metodě `Image::saveToFile()`. Tato metoda dává pozor na originální strukturu BMP souboru a zachovává všechny důležité informace:

```cpp
bool Image::saveToFile(const QString &filePath) const {
    // Kontrola, zda je obrázek prázdný
    if (isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // 1. Zápis file header (14 bajtů)
    file.write(fileHeader.bfType, 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfSize), 4);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfReserved1), 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfReserved2), 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfOffBits), 4);

    // 2. Zápis info header (40 bajtů)
    file.write(reinterpret_cast<const char*>(&infoHeader.biSize), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biWidth), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biHeight), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biPlanes), 2);
    file.write(reinterpret_cast<const char*>(&infoHeader.biBitCount), 2);
    file.write(reinterpret_cast<const char*>(&infoHeader.biCompression), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biSizeImage), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biXPelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biYPelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biClrUsed), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biClrImportant), 4);

    // 3. Zápis palety barev (pokud existuje)
    if (imageBitsPerPixel <= 8 && !colorPalette.isEmpty()) {
        for (QRgb color : colorPalette) {
            char paletteEntry[4];
            paletteEntry[0] = static_cast<char>(qBlue(color));  // B
            paletteEntry[1] = static_cast<char>(qGreen(color)); // G
            paletteEntry[2] = static_cast<char>(qRed(color));   // R
            paletteEntry[3] = 0; // Reserved
            file.write(paletteEntry, 4);
        }
    }

    // 4. Zápis obrazových dat
    if (modified) {
        // Výpočet velikosti řádku (musí být zarovnán na 4 bajty)
        int bytesPerRow = calculateRowSize();
        QByteArray dataToSave(bytesPerRow * imageHeight, 0);

        // Konverze pixelů z QImage zpět do formátu BMP
        for (int y = 0; y < qImage.height(); y++) {
            for (int x = 0; x < qImage.width(); x++) {
                QRgb pixel = qImage.pixel(x, y);

                // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
                int row = (infoHeader.biHeight > 0) ? imageHeight - 1 - y : y;
                int byteIndex = row * bytesPerRow;

                // Logika ukládání podle bitové hloubky
                // ...
            }
        }
        file.write(dataToSave);
    } else {
        // Použití původních dat, pokud obrázek nebyl upraven
        file.write(rawData);
    }

    file.close();
    return true;
}
```

Metoda optimalizuje ukládání:
1. Pokud obrázek nebyl modifikován, ukládá původní raw data, což zajišťuje zachování originálu
2. Pro modifikované obrázky vytváří nová data podle aktuálního obsahu v CustomImageWidget
3. Respektuje formát dat podle bitové hloubky (24, 8, 4 nebo 1 bit)
4. Pro obrázky s paletou (8, 4, 1 bit) hledá nejbližší barvu v paletě pro každý pixel

## 5. Implementace filtrů

Aplikace využívá návrhový vzor Strategy pro implementaci obrazových filtrů. Základní třída `Filter` definuje rozhraní:

```cpp
class Filter {
public:
    virtual ~Filter() = default;
    virtual QImage apply(const QImage& image) const = 0;
    virtual QString name() const = 0;
};
```

Z této třídy dědí konkrétní implementace filtrů:
- **InvertFilter**: Invertuje barvy každého pixelu
- **RotateFilter**: Rotuje obrázek o 90 stupňů
- **FlipFilter**: Horizontálně převrací obrázek

## 6. Uživatelské rozhraní

Hlavní okno aplikace je rozděleno na tři části:
1. Panel s filtry v horní části
2. Plocha pro zobrazení obrázku uprostřed
3. Panel s informacemi a ovládáním zoomu vpravo

Aplikace obsahuje také hlavní menu s možnostmi pro otevření a uložení souborů.

## 7. Závěr

Tato aplikace je jednoduchý nástroj pro zobrazování a úpravu nekomprimovaných obrázků ve formátu BMP s rozmezím barevných hloubek 1, 4, 8 a 24 bitů na pixel.

Klíčové stránky implementace:
- Podpora různých variant formátu BMP (1, 4, 8 a 24 bitů na pixel)
- Využití vlastních funkcí pro vykreslení
- Práce s headery, paletami a raw daty
