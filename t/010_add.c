#include <stdio.h>
#include "timer.h"
#include "bigint.h"

#define ALEN(a) (int) ((sizeof(a) / sizeof((a)[0])))

static void test_add(void) {
    static struct {
        const char* l;
        const char* r;
        const char* e;
    } data[] = {
        // really basic shit
        {    "0",    "0",     "0" },
        {    "0",    "1",     "1" },
        {    "1",    "0",     "1" },
        {    "1",    "1",     "2" },
        {   "-1",   "-1",    "-2" },
        // {    "1",   "-1",     "0" }, TODO this aborts
        // {   "-1",    "1",     "0" }, TODO this aborts
        // small shit
        { "12345", "67890", "80235" },
        // huge shit
        {
          "12345678901234567890",
               "234567890123456",
          "12345913469124691346"
        },
        {
          "99999999999999999999",
               "999999999999999",
         "100000999999999999998",
        },
        // generated with bc
  /*
  echo "r=$RANDOM ^ 7; n = r; for (j=0; j<20; ++j) { a = n; n += r; b = n; n += r; c = a + b; print \"\na \", a, \"\nb \", b, \"\nc \", c, \"\n\";}" | bc -q | sed 's+\n++g' | awk '$1 ~ /a/ {printf("        { @%s@,", $2);} $1 ~ /b/ {printf("@%s@,", $2);} $1 ~ /c/ {printf("@%s@ },\n", $2);}' | tr '@' '"'
*/
        {
          "75750280597168309002282112",
          "151500561194336618004564224",
          "227250841791504927006846336",
        },
        {
          "227250841791504927006846336",
          "303001122388673236009128448",
          "530251964180178163015974784",
        },
        {
          "378751402985841545011410560",
          "454501683583009854013692672",
          "833253086568851399025103232",
        },
        {
          "530251964180178163015974784",
          "606002244777346472018256896",
          "1136254208957524635034231680",
        },
        {
          "681752525374514781020539008",
          "757502805971683090022821120",
          "1439255331346197871043360128",
        },
        {
          "833253086568851399025103232",
          "909003367166019708027385344",
          "1742256453734871107052488576",
        },
        {
          "984753647763188017029667456",
          "1060503928360356326031949568",
          "2045257576123544343061617024",
        },
        {
          "1136254208957524635034231680",
          "1212004489554692944036513792",
          "2348258698512217579070745472",
        },
        {
          "1287754770151861253038795904",
          "1363505050749029562041078016",
          "2651259820900890815079873920",
        },
        {
          "1439255331346197871043360128",
          "1515005611943366180045642240",
          "2954260943289564051089002368",
        },
        {
          "1590755892540534489047924352",
          "1666506173137702798050206464",
          "3257262065678237287098130816",
        },
        {
          "1742256453734871107052488576",
          "1818006734332039416054770688",
          "3560263188066910523107259264",
        },
        {
          "1893757014929207725057052800",
          "1969507295526376034059334912",
          "3863264310455583759116387712",
        },
        {
          "2045257576123544343061617024",
          "2121007856720712652063899136",
          "4166265432844256995125516160",
        },
        {
          "2196758137317880961066181248",
          "2272508417915049270068463360",
          "4469266555232930231134644608",
        },
        {
          "2348258698512217579070745472",
          "2424008979109385888073027584",
          "4772267677621603467143773056",
        },
        {
          "2499759259706554197075309696",
          "2575509540303722506077591808",
          "5075268800010276703152901504",
        },
        {
          "2651259820900890815079873920",
          "2727010101498059124082156032",
          "5378269922398949939162029952",
        },
        {
          "2802760382095227433084438144",
          "2878510662692395742086720256",
          "5681271044787623175171158400",
        },
        {
          "2954260943289564051089002368",
          "3030011223886732360091284480",
          "5984272167176296411180286848",
        },
        {
          "43745172862066215515797577363",
          "87490345724132431031595154726",
          "131235518586198646547392732089",
        },
        {
          "131235518586198646547392732089",
          "174980691448264862063190309452",
          "306216210034463508610583041541",
        },
        {
          "218725864310331077578987886815",
          "262471037172397293094785464178",
          "481196901482728370673773350993",
        },
        {
          "306216210034463508610583041541",
          "349961382896529724126380618904",
          "656177592930993232736963660445",
        },
        {
          "393706555758595939642178196267",
          "437451728620662155157975773630",
          "831158284379258094800153969897",
        },
        {
          "481196901482728370673773350993",
          "524942074344794586189570928356",
          "1006138975827522956863344279349",
        },
        {
          "568687247206860801705368505719",
          "612432420068927017221166083082",
          "1181119667275787818926534588801",
        },
        {
          "656177592930993232736963660445",
          "699922765793059448252761237808",
          "1356100358724052680989724898253",
        },
        {
          "743667938655125663768558815171",
          "787413111517191879284356392534",
          "1531081050172317543052915207705",
        },
        {
          "831158284379258094800153969897",
          "874903457241324310315951547260",
          "1706061741620582405116105517157",
        },
        {
          "918648630103390525831749124623",
          "962393802965456741347546701986",
          "1881042433068847267179295826609",
        },
        {
          "1006138975827522956863344279349",
          "1049884148689589172379141856712",
          "2056023124517112129242486136061",
        },
        {
          "1093629321551655387894939434075",
          "1137374494413721603410737011438",
          "2231003815965376991305676445513",
        },
        {
          "1181119667275787818926534588801",
          "1224864840137854034442332166164",
          "2405984507413641853368866754965",
        },
        {
          "1268610012999920249958129743527",
          "1312355185861986465473927320890",
          "2580965198861906715432057064417",
        },
        {
          "1356100358724052680989724898253",
          "1399845531586118896505522475616",
          "2755945890310171577495247373869",
        },
        {
          "1443590704448185112021320052979",
          "1487335877310251327537117630342",
          "2930926581758436439558437683321",
        },
        {
          "1531081050172317543052915207705",
          "1574826223034383758568712785068",
          "3105907273206701301621627992773",
        },
        {
          "1618571395896449974084510362431",
          "1662316568758516189600307939794",
          "3280887964654966163684818302225",
        },
        {
          "1706061741620582405116105517157",
          "1749806914482648620631903094520",
          "3455868656103231025748008611677",
        },
    };

    bigint* l = bigint_create();
    bigint* r = bigint_create();
    bigint* g = bigint_create();
    bigint* e = bigint_create();
    Timer t;
    for (int j = 0; j < ALEN(data); ++j) {
        bigint_assign_string(l, data[j].l);
        bigint_assign_string(r, data[j].r);
        timer_start(&t);
        bigint_add(l, r, g);
        timer_stop(&t);

        bigint_assign_string(e, data[j].e);
        int ok = bigint_compare(g, e) == 0;
        printf("%-3s [%s] -- ", ok ? "OK" : "XX", data[j].e);
        timer_format_elapsed(&t, stdout, 1);
    }
    bigint_destroy(g);
    bigint_destroy(e);
    bigint_destroy(r);
    bigint_destroy(l);
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    test_add();

    return 0;
}
