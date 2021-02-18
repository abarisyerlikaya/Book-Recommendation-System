// ------------------------ ON ISLEMCI TANIMLARI ------------------------

#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "RecomendationDataSet.csv"
#define BUFFER_SIZE 1000
#define NAME_LENGTH 50
#define PRED_STR_LENGTH 100
#define MAX_BOOKS 80
#define MAX_USERS 250

// ------------------------ FONKSIYON PROTOTIPLERI ------------------------

// Kullanicidan userName stringini ve k degerini okur ve bu degerleri parametre olarak girilen degiskenlere yazar
void interact(char userName[], int* k, int userCount, int testUsersOffset);

// userIndex numarali kullanicinin bookIndex numarali kitap icin verecegi tahmini puani hesaplar ve return eder
float predictBookRating(float similarity[][MAX_USERS], int userIndex, int* kClosest, int k, int ratings[][MAX_BOOKS], int bookIndex, int bookCount);

// Girilen predictionCount eleman sayili predictions dizisini, pred degerlerine gore buyukten kucuge siralar
void sortPredictions(char* predictions[], int predictionCount);

// Bir kullaniciya ait benzerilkleri tutan similarityRow dizisi ve k degerine gore, kullaniciya en yakin k kullaniciyi tutan diziyi bulur ve return eder
int* findKClosest(float similarityRow[], int k, int testUsersOffset);

// Girilen kClosest dizisindeki en kucuk degerli delemani similarityRow'a gore bulur ve bu elemanin indexini return eder
int findMinimum(int kClosest[], float similarityRow[], int k);

// Girilen userIndex1 ve userIndex2 numarali iki kullanicinin benzerligini rating matrisine gore Pearson Coeff. yontemi ile bulur ve return eder
float findSimilarity(int ratings[][MAX_BOOKS], int userIndex1, int userIndex2, int bookCount);

// CSV dosyasindan kitap isimleri, kullanici isimleri, puanlar, kitap sayisi, ve NU'lu kullanicilarin basladigi indexi okur ve degiskenlere yazar
void getDataFromFile(char* books[], char* users[], int ratings[][MAX_BOOKS], int* userCount, int* testUsersOffset, int* bookCount);

// Verilen CSV dosayaina ait bir satir olan string'in tokenize edilebilmesi icin bazi duzeltmeler yapar (";;" -> ";0;") (";\n\0" -> ";0\0")
void parseLine(char* string);

// ------------------------ MAIN FONKSIYONU ------------------------

int main() {
    char* books[MAX_BOOKS];                  // Kitap isimleri
    char* users[MAX_USERS];                  // Kullanici isimleri
    int ratings[MAX_USERS][MAX_BOOKS];       // Kullanicilarin kitaplara verdigi puanlar (Orn: ratings[i][j] => i'nin j kitabina verdigi puan)
    float similarity[MAX_USERS][MAX_USERS];  // Kullanicilarin benzerlikleri (Orn: similarity[i][j] => i'nin j ile olan benzerligi)
    char* predictions[MAX_BOOKS];            // Prediction degerlerini kitap isimleriyle birlikte tutan string dizisi (Orn: "THE DA VINCI CODE: 1.9308")
    int* kClosest;                           // Girilen kullaniciye en yakin k kullanicinin indexini tutan dizi
    int userCount;                           // Toplam kullanici sayisi
    int testUsersOffset;                     // NU kullanicilarinin basladigi index
    int bookCount;                           // Toplam kitap sayisi
    int predictionCount;                     // Toplam tahmin sayisi (predictions'in boyutu)
    int k;                                   // En yakin kac kullanici olacak? (kClosest'in boyutu)
    char userName[NAME_LENGTH];              // Input olarak girilen kullanici adi
    int userIndex;                           // Input olarak girilen kullanicinin indexi
    int i;                                   // Dongu degiskeni
    int j;                                   // Dongu degiskeni
    char key = ' ';                          // Kullanicinin islem sonunda bastigi tus ('E' ise programi kapatacak)

    // RecomendationDataSet.csv dosyasindan verileri oku ve degiskenlere yaz
    getDataFromFile(books, users, ratings, &userCount, &testUsersOffset, &bookCount);

    // Her bir kullanicinin diger tum kullanicilarla olan benzerligini hesapla
    for (i = 0; i < userCount; i++)
        for (j = 0; j < userCount; j++)
            similarity[i][j] = findSimilarity(ratings, i, j, bookCount);

    // Kullanici 'E' tusuna basmadigi surece tekrar input al
    while (key != 'E' && key != 'e') {
        interact(userName, &k, userCount, testUsersOffset);                  // Kullanicidan inputlari al
        userIndex = testUsersOffset + atoi(&userName[2]) - 1;                // Input olarak girilen kullanici adina gore kullanci indexini bul
        kClosest = findKClosest(similarity[userIndex], k, testUsersOffset);  // Bulunan indexteki kullaniciya en yakin k kisiyi bul

        predictionCount = 0;  // Tahmin sayisini sifirla

        // Kullanicinin okudugu tum kitaplar icin birer defa don
        for (i = 0; i < bookCount; i++) {
            // Eger kullanici bu kitabi okumadiysa
            if (ratings[userIndex][i] == 0) {
                predictions[predictionCount] = (char*)malloc(sizeof(char) * PRED_STR_LENGTH);              // Prediction stringi icin yer ayir
                float tmp = predictBookRating(similarity, userIndex, kClosest, k, ratings, i, bookCount);  // Kitaba verilecek tahmini puani hesapla
                sprintf(predictions[predictionCount], "%-20s: %5.4f", books[i], tmp);                      // Prediction stringini olustur
                predictionCount++;                                                                         // Tahmin sayisini bir arttir
            }                                                                                              // Prediction stringi orn: "RUNNY BABBIT: 1.7506"
        }

        sortPredictions(predictions, predictionCount);  // Tahminleri buyukten kucuge sirala

        // En yakin k kullaniciyi ve benzerlik degerlerini yazdir
        printf("%s kullanicisina en yakin kullanicilar (k=%d) ve hesaplanan pearson benzerlikleri:\n", userName, k);
        for (i = 0; i < k; i++)
            printf("%-5s: %5.3f\n", users[kClosest[i]], similarity[userIndex][kClosest[i]]);

        // Kullanicinin okumadigi kitaplar icin tahmini puanlari ve onerilen kitabi yazdir
        printf("\n%s kullanicisinin okumadigi kitaplar icin hesaplanan tahmini puanlar:\n", userName);
        for (i = 0; i < predictionCount; i++)
            printf("%s\n", predictions[i]);
        printf("\nSonuc olarak onerilen kitap:\n%s", strtok(predictions[0], ":"));

        printf("\n\nCikmak icin 'E' tusuna basiniz. Devam etmek icin baska bir tusa basiniz...");
        key = getch();
    }

    return 0;
}

// ------------------------ FONKSIYON TANIMLARI ------------------------

void interact(char userName[], int* k, int userCount, int testUsersOffset) {
    system("CLS");  // Ekrani temizle

    // Bilgilendirme mesaji yazdir
    printf("-------------------- KITAP ONERI SISTEMI --------------------\n\n");
    printf("Bu program, RecomendationDataSet.csv dosyasindan bilgileri okuyarak U1, U2 ..., U[i] okuyucularinin kitaplara\n");
    printf("verdigi puanlari kullanarak NU1, NU2 ..., NU[j] kullanicilarinin hangi kitaba kac puan verecegini tahmin eder\n");
    printf("ve bu tahminler arasindan en yakin buldugu kitabi kullaniciya onerir.\n\n");
    printf("DIKKAT: RecomendationDataSet.csv dosyasi uygun formatta degilse program calismayabilir.\n\n");

    // Oneri yapilacak kullanicinin adini al
    printf("Lutfen kitap onerilmesini istediginiz okuyucunun kullanici adini giriniz: (Ornek: NU1, NU2 ..., NU[n])\n");
    scanf("%s", userName);
    while (userName[0] != 'N' || userName[1] != 'U' || testUsersOffset + atoi(&userName[2]) - 1 >= userCount || atoi(&userName[2]) <= 0) {
        printf("\nLutfen gecerli bir kullanici adi giriniz: (Ornek: NU1, NU2 ..., NU[n])\n");
        scanf("%s", userName);
    }

    // Hesaplamada kullanilacak benzer kullanici sayisini al
    printf("\nLutfen benzer kullanici sayisini (k) giriniz: (1 <= k <= Hesaplamada kullanilan kullanicilarinin sayisi)\n");
    scanf("%d", k);
    while (*k <= 0 || *k > testUsersOffset) {
        printf("\nLutfen gecerli bir k degeri giriniz: (1 <= k <= Hesaplamada kullanilan kullanicilarinin sayisi)\n");
        scanf("%d", k);
    }
    printf("\n");
}

float predictBookRating(float similarity[][MAX_USERS], int userIndex, int* kClosest, int k, int ratings[][MAX_BOOKS], int bookIndex, int bookCount) {
    float mean1 = 0;        // Formuldeki ra degeri (Bir kere hesaplanacak)
    float mean2 = 0;        // Formuldeki rb degeri (Dongu icinde surekli degisecek)
    int ratingCount1 = 0;   // ra = toplam puan / puanlama sayisi => buradaki puanlama sayisini tutan degisken
    int ratingCount2 = 0;   // rb = toplam puan / puanlama sayisi => buradaki puanlama sayisini tutan degisken
    float numerator = 0;    // pred = ra + (pay / payda) => buradaki pay degerini tutan degisken
    float denominator = 0;  // pred = ra + (pay / payda) => buradaki payda degerini tutan degisken
    int i, j;               // Dongu degiskenleri

    // A kullanicisinin okudugu kitaplara verdigi ortalama puani hesapla
    for (i = 0; i < bookCount; i++) {
        if (ratings[userIndex][i] != 0) {
            mean1 += ratings[userIndex][i];
            ratingCount1++;
        }
    }
    mean1 /= ratingCount1;

    // A kullanicisina en yakin k kullanici icin birer defa don
    for (i = 0; i < k; i++) {
        mean2 = 0;         // Ortalama degerini sifirla
        ratingCount2 = 0;  // Puanlama sayisini sifirla

        // B kullanicisinin okudugu kitaplara verdigi ortalama puani hesapla
        for (j = 0; j < bookCount; j++) {
            if (ratings[kClosest[i]][j] != 0) {
                mean2 += ratings[kClosest[i]][j];
                ratingCount2++;
            }
        }
        mean2 /= ratingCount2;

        // Pay ve paydayi guncelle
        numerator += similarity[userIndex][kClosest[i]] * (ratings[kClosest[i]][bookIndex] - mean2);
        denominator += similarity[userIndex][kClosest[i]];
    }

    // Sonucu yani ra + (pay / payda) degerini dondur
    return mean1 + (numerator / denominator);
}

void sortPredictions(char* predictions[], int predictionCount) {
    char* token;    // Strtok sonucu donen token'i tutacak olan string
    char* backup1;  // Orijinal string, tokenize edilmeden once bu stringde yedeklenir ve islem sonunda orijinal stringe buradan kopyalanir
    char* backup2;  // Orijinal string, tokenize edilmeden once bu stringde yedeklenir ve islem sonunda orijinal stringe buradan kopyalanir
    float key;      // Insertion sort'ta kullanilacak key degiskeni
    float tmp;      // Gecici degisken
    int i, j;       // Dongu degiskenleri

    // Insertion sort dis dongu (1 indexli prediction'dan baslar, son prediction'a kadar doner)
    for (i = 1; i < predictionCount; i++) {
        backup1 = strdup(predictions[i]);     // Stringi tokenize etmeden once yedek stringe kopyala
        token = strtok(predictions[i], ":");  // ':' karakterine gore tokenize et, ilk token'i al (kitap ismi)
        token = strtok(NULL, ":");            // Devam edip ikinci token'i al (kitabin tahmini puani)
        predictions[i] = backup1;             // Onceden alinan yedegi, orijinal stringe geri ata

        key = atof(token);  // Su anki kitabin puanini key'e ata
        j = i - 1;          // j degerini i - 1'den baslat

        backup2 = strdup(predictions[j]);     // Stringi tokenize etmeden once yedek stringe kopyala
        token = strtok(predictions[j], ":");  // ':' karakterine gore tokenize et, ilk token'i al (kitap ismi)
        token = strtok(NULL, ":");            // Devam edip ikinci token'i al (kitabin tahmini puani)
        predictions[j] = backup2;             // Onceden alinan yedegi, orijinal stringe geri ata
        tmp = atof(token);                    // Key ile karsilastirilacak kitabin puanini tmp'ye ata

        // Insertion sort ic dongu (Su anki prediction'dan geriye dogru 0 indexli prediction'a kadar; prediction, key'den kucuk oldugu surece doner)
        while (j >= 0 && tmp < key) {
            predictions[j + 1] = predictions[j];  // Yer degistir

            // Uc case kontrolu, 0'in altina duserse islem yapilmasin diye
            if (--j >= 0) {
                backup2 = strdup(predictions[j]);     // Stringi tokenize etmeden once yedek stringe kopyala
                token = strtok(predictions[j], ":");  // ':' karakterine gore tokenize et, ilk token'i al (kitap ismi)
                token = strtok(NULL, ":");            // Devam edip ikinci token'i al (kitabin tahmini puani)
                predictions[j] = backup2;             // Onceden alinan yedegi, orijinal stringe geri ata
                tmp = atof(token);                    // Key ile karsilastirilacak kitabin puanini tmp'ye ata
            }
        }
        predictions[j + 1] = backup1;  // Donguden ciktiginda buldugu yere (0 ya da key'den buyuk bir prediction) key'i atar
    }
}

int* findKClosest(float similarityRow[], int k, int testUsersOffset) {
    int* kClosest = (int*)malloc(sizeof(int) * k);  // Kullaniciya ait similarityRow'a gore en yakin k kisinin indexini tutacak dizi
    int indexOfMin;                                 // kClosest dizisindeki en az benzer olan elemaninin indexini tutacak
    int sortKey;                                    // Insertion sort'ta kullanilacak olan key
    int i, j;                                       // Dongu degiskenleri

    // Once ilk k kullaniciyi en yakin k kullanici olarak kabul et
    for (i = 0; i < k; i++)
        kClosest[i] = i;

    indexOfMin = findMinimum(kClosest, similarityRow, k);  // Bu k kullanici arasindan en az benzer olani bul

    // Her bir U[k], U[k+1] ..., U[n] kullanicisi icin birer defa donecek
    for (i = k; i < testUsersOffset; i++) {
        // Eger kullanici, en benzer k kullanici arasindan en az benzer olanindan daha benzerse
        if (similarityRow[i] > similarityRow[kClosest[indexOfMin]]) {
            kClosest[indexOfMin] = i;                              // En az benzer olan yerine bu kullaniciyi yerlestir
            indexOfMin = findMinimum(kClosest, similarityRow, k);  // Dizi degisti, en az benzer olan kullaniciyi tekrar bul
        }
    }

    // Insertion sort ile en benzer k kullaniciyi buyukten kucuge sirala
    for (i = 1; i < k; i++) {
        sortKey = kClosest[i];
        for (j = i - 1; j >= 0 && similarityRow[kClosest[j]] < similarityRow[sortKey]; j--)
            kClosest[j + 1] = kClosest[j];
        kClosest[j + 1] = sortKey;
    }

    return kClosest;  // En benzer k kullanicinin indexlerini tutan diziyi return et
}

int findMinimum(int* kClosest, float similarityRow[], int k) {
    int indexOfMin = 0;  // Minimum degerli elemanin indexi, 0 olarak ilklendirildi
    int i;               // Dongu degiskeni

    // 0, min olarak ilklendirildigi icin 1 numarali indexten k'ya kadar donecek
    for (i = 1; i < k; i++)
        if (similarityRow[kClosest[i]] < similarityRow[kClosest[indexOfMin]])
            indexOfMin = i;  // Eger daha once belirlenmis minimumdan kucukse yeni minimum indexi olarak su anki elemanin indexini ata

    return indexOfMin;  // Sonucu return et
}

float findSimilarity(int ratings[][MAX_BOOKS], int userIndex1, int userIndex2, int bookCount) {
    float mean1 = 0;       // Formuldeki ra degeri (Dongu icinde surekli degisecek)
    float mean2 = 0;       // Formuldeki rb degeri (Dongu icinde surekli degisecek)
    int ratingCount1 = 0;  // ra = toplam puan / puanlama sayisi => buradaki puanlama sayisini tutan degisken
    int ratingCount2 = 0;  // rb = toplam puan / puanlama sayisi => buradaki puanlama sayisini tutan degisken
    float cov = 0;         // sim = cov(a,b) / (std(a) * std(b)) => buradaki cov degeri
    float std1 = 0;        // sim = cov(a,b) / (std(a) * std(b)) => buradaki std(a) degeri
    float std2 = 0;        // sim = cov(a,b) / (std(a) * std(b)) => buradaki std(b) degeri
    int i;                 // Dongu degiskeni

    // A ve B kullanicilarinin okuduklari kitaplara verdikleri ortalama puanlari hesapla
    for (i = 0; i < bookCount; i++) {
        if (ratings[userIndex1][i] != 0) {
            mean1 += ratings[userIndex1][i];
            ratingCount1++;
        }
        if (ratings[userIndex2][i] != 0) {
            mean2 += ratings[userIndex2][i];
            ratingCount2++;
        }
    }
    mean1 /= ratingCount1;
    mean2 /= ratingCount2;

    // Her kitap icin bir kere don
    for (i = 0; i < bookCount; i++) {
        // Eger bu kitabi hem A hem de B kullanicisi okuduysa
        if (ratings[userIndex1][i] != 0 && ratings[userIndex2][i] != 0) {
            float diff1 = ratings[userIndex1][i] - mean1;  // A'nin verdigi puanin, ortalamasina olan farkini bul
            float diff2 = ratings[userIndex2][i] - mean2;  // B'nin verdigi puanin, ortalamasina olan farkini bul
            cov += diff1 * diff2;                          // cov(a,b) degerini guncelle
            std1 += diff1 * diff1;                         // std(a) degerini guncelle
            std2 += diff2 * diff2;                         // std(B) degerini guncelle
        }
    }

    // Sonucu hesapla ve return et
    std1 = sqrt(std1);
    std2 = sqrt(std2);
    return cov / (std1 * std2);
}

void getDataFromFile(char* books[], char* users[], int ratings[][MAX_BOOKS], int* userCount, int* testUsersOffset, int* bookCount) {
    FILE* fp = NULL;           // Dosyayi gosterecek olan file pointer
    char buffer[BUFFER_SIZE];  // Dosyadan okunacak olan her bir satirin gecici olarak tutulacagi string
    char* token;               // Strtok sonucu donen tokenin gecici olarak tutulacagi string
    int i = 0;                 // Dongu degiskeni
    int j = 0;                 // Dongu degiskeni

    fp = fopen(FILE_NAME, "r");  // RecomendationDataSet.csv dosyasini okuma modunda ac

    // Dosya acilamadiysa hata mesaji ver ve cik
    if (!fp) {
        printf("HATA: Dosya acilamadi, lutfen dosya formatinizi kontrol ediniz!");
        return;
    }

    printf("Veriler okunuyor...\n");  // Acildiysa verileri okumaya baslamadan once bilgi mesaji ver

    fgets(buffer, BUFFER_SIZE, fp);  // Ilk satiri buffer'a al, bu satir kitap isimlerini iceriyor
    token = strtok(buffer, ";");     // ';' karakterine gore tokenize et ve ilk ';' karakterinden onceki kismi gec
    token = strtok(NULL, ";");       // Ilk ';' karakterinden sonraki kismi token'a al (Ilk kitap ismi bu)

    // ';' karakterine gore ayrilmis tum token'lari tek tek gez (her token bir kitap ismi)
    while (token != NULL) {
        if (token[strlen(token) - 1] == '\n')
            token[strlen(token) - 1] = '\0';  // Kitap isminin duzgun olmasi icin sonunda '\n' karakteri varsa o karakteri sil
        books[i] = strdup(token);             // Su anki token'i kitaplar dizisinin i. indexine yerlestir
        token = strtok(NULL, ";");            // Bir sonraki token'a gec
        i++;                                  // Kitaplar dizisinin bir sonraki elemanina gec
    }

    *bookCount = i;  // i kadar donduyse i tane kitap vardir, bookCount degiskenine i'yi ata

    fgets(buffer, BUFFER_SIZE, fp);  // Ikınci satiri buffer'a al, bu satirdan itibaren her satirin ilk tokeni kullanici isimlerini iceriyor
    i = 0;                           // Yeni donguye girmeden once i'yi sifirla

    // Dosyadaki tum satirlari tek tek gez (her satirin ';' karakterine gore ayrilmis ilk tokeni bir kullanici ismi)
    while (!feof(fp)) {
        token = strtok(buffer, ";");     // Su anki satirin ';' karakterine gore ayrilmis ilk tokenini al
        users[i] = strdup(token);        // Bu tokeni kullanicilar dizisinin i. indexine yerlestir
        fgets(buffer, BUFFER_SIZE, fp);  // Bir sonraki satiri buffer'a al
        i++;                             // Kullanicilar dizisinin bir sonraki elemanina gec
    }

    *userCount = i;                                          // i kadar donduyse i tane kullanici vardir, userCount degiskenine i'yi ata
    *testUsersOffset = *userCount - atoi(&users[i - 1][2]);  // Son karakter NUx ise userCount - x, testUsersOffset'i belirler

    fseek(fp, 0, SEEK_SET);          // Dosyanin basina don
    fgets(buffer, BUFFER_SIZE, fp);  // Ilk satiri buffer'a al
    i = 0;                           // Donguye girmeden once i'yi sifirla

    // Ikinci satirdan itibaren her bir satir icin
    while (!feof(fp)) {
        fgets(buffer, BUFFER_SIZE, fp);  // Satiri buffer'a al
        parseLine(buffer);               // Satiri sorunsuz tokenize edebilmek icin duzelt

        token = strtok(buffer, ";");  // ';' karakterine gore ilk tokeni gec
        token = strtok(NULL, ";");    // ';' karakterine gore ikinci tokeni al (buradan itibaren puanlar basliyor)
        j = 0;                        // Donguye girmeden once j'yi sifirla

        // Satirdaki ikinci tokenden itibaren tum tokenler icin birer defa don (her token bir puani gosteriyor)
        while (token != NULL) {
            ratings[i][j] = atoi(token);  // i kullanicisinin j kitabi icin verdigi puani oku ve ratings matrisine yerlestir
            token = strtok(NULL, ";");    // Bir sonraki tokena yani kitaba gec
            j++;                          // Ratings matrisinde bir sonraki sutuna yani kitaba gec
        }

        i++;  // Bir sonraki satira yani bir sonraki kullaniciya gecilecek, ratings matrisinde de bir sonraki satira yani kullaniciya gec
    }
}

void parseLine(char* string) {
    char* found = strstr(string, ";;");  // Satirda ";;" substringini ara, bulunduysa baslangic adresi found'da olacak
    int length;                          // Islemin sonunda elde edilen stringin uzunlugunu tutacak degisken

    // Satirda bulunan her ";;" substringi icin birer defa donecek
    while (found != NULL) {
        memmove(found + 2, found + 1, strlen(found) + 1);  // [';'][';']        ->  [';'][ ? ][';']
        found[1] = '0';                                    // [';'][ ? ][';']   ->  [';']['0'][';'] yani => ";;" -> ";0;"
        found = strstr(found, ";;");                       // Bir sonraki ";;" substringini ara
    }

    length = strlen(string);        // Stringin su anki uzunlugunu length'e al
    if (string[length - 2] == ';')  // Stringin sondan ikinci karakteri ';' ise
        string[length - 1] = '0';   // Sondan birinci karakterini sifir yap yani => ";\0" -> ";0\0"
}