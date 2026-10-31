// Self-check for Biscuit's content: personalize(), the generated art tables, and the pixel gate that every string
// fits the box its screen draws it in (games/biscuit/layout.h) and stays on the round glass. Run: make test
// (Glyphs, tokens, ids and order are checked by games/biscuit/tools/check_content.py; art pixels by the art tool's --check.)
#include "host/check.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "crc32.h"
#include "games/biscuit/content_daily.h"
#include "games/biscuit/content_discoveries.h"
#include "games/biscuit/content_stories.h"
#include "games/biscuit/generated/discovery_art.h"
#include "games/biscuit/generated/scenes.h"
#include "games/biscuit/layout.h"
#include "games/biscuit/personalize.h"
#include "games/biscuit/pet.h"
#include "gfx565.h"
#include "packed_image.h"

using namespace biscuit;

static void personalizing() {
  char out[64];
  assert(personalize("{pet} nudged {name}'s hand.", "Sam", "Pip", out, sizeof out) == 22);
  assert(strcmp(out, "Pip nudged Sam's hand.") == 0);
  personalize("Night, {name}. Love, {pet}", "", nullptr, out, sizeof out);   // no names yet
  assert(strcmp(out, "Night, Friend. Love, Biscuit") == 0);
  personalize("{named} {pe} {}", "Sam", "Pip", out, sizeof out);             // only whole tokens are filled
  assert(strcmp(out, "{named} {pe} {}") == 0);
  assert(personalize("{name}{name}{name}", "Alexandra", "Pip", out, 10) == 9);  // cut, never overrun
  assert(strcmp(out, "Alexandra") == 0);
  assert(personalize("x", "Sam", "Pip", out, 0) == 0);
}

// Every public art reference still has its original dimensions and a nonempty packed payload.
// Pixel identity, decoding validity and all 504 scene references are exercised below.
static void art() {
  static_assert(sizeof DISCOVERIES / sizeof DISCOVERIES[0] == DISCOVERY_COUNT, "one picture per discovery");
  for (const packed565::Image& picture : DISCOVERY_ART) {
    assert(picture.width == DISCOVERY_W && picture.height == DISCOVERY_H);
    assert(picture.data && picture.byteCount > 0);
  }
  for (const auto& stage : SCENES)
    for (const auto& time : stage)
      for (const auto& activity : time)
        for (const packed565::Image& scene : activity) {
          assert(scene.width == SCENE_W && scene.height == SCENE_H);
          assert(scene.data && scene.byteCount > 0);
        }
}

// ---- the pixel gate: os/font.cpp measures and draws each string exactly as the device will ----
// Tokens are filled with the widest names a kid can end up with: 11 letters for the profile name (its record holds
// 11), NAME_LIMIT for the dog's, every letter a W. Any case is possible (a migrated Biscuit save keeps its names as
// typed), W is the widest letter in every Biscuit font (checked below), and letters never kern in them, so no name
// of that length is wider. Greedy wrapping never gets shorter when a word gets wider, so this is the worst case.
static const char NAME[] = "WWWWWWWWWWW", PET[] = "WWWWWWWWWWWW";
static_assert(sizeof NAME == 12 && sizeof PET == NAME_LIMIT + 1, "the longest names");
static uint16_t g_fb[gfx565::W * gfx565::H];
static int failures = 0;

// Every draw must write the newly selected panel buffer, even when the same asset was just drawn.
static void secondTarget(const packed565::Image& image, int x, int y, int scale) {
  static uint16_t second[gfx565::W * gfx565::H];
  gfx565::target(second);
  gfx565::clear(0);
  assert(packed565::draw(image, x, y, scale));
  assert(memcmp(g_fb, second, sizeof second) == 0);
  gfx565::target(g_fb);
}

// Frozen from the original uncompressed RGB565 arrays, independently checked against all 96 legacy SHA-256
// manifest entries. Exercise the actual renderer at list (1x) and cover (3x) scales, including every expanded pixel.
static void discoveryPixels() {
  static const uint32_t originalCrc[DISCOVERY_COUNT] = {
    0x55E833B9u, 0xB70B05BEu, 0xABA73DFCu, 0x9C38F67Du, 0xF8AAA38Au, 0x2E452652u, 0x43B42E32u, 0x409B2EC5u,
    0xC9C452DCu, 0xAABD9A75u, 0xA7B178E0u, 0xBAE496B5u, 0x7B410885u, 0xB8278F7Au, 0x0FE7961Cu, 0x0847180Fu,
    0x3AFC42DCu, 0xC9AF002Du, 0x2111C491u, 0x85A5828Bu, 0x2A8E4D3Fu, 0xA56A0D31u, 0xF1AB34FBu, 0xB676D381u,
    0x532D971Eu, 0x9D9AD7CFu, 0x3CFBF904u, 0xD0CC101Fu, 0x20E892E1u, 0xFA0203A4u, 0x68FAB573u, 0x465520E8u,
    0x82940995u, 0xCEA1B340u, 0x15D1950Eu, 0xC6B741B7u, 0x4A9FE9C2u, 0x4ECCAC1Du, 0x3C758927u, 0x3FAC9274u,
    0xDB2ED57Bu, 0x13F10A1Au, 0x275FD886u, 0x376E6451u, 0x2D2B329Au, 0xFF96F5D1u, 0xC82BDC45u, 0x5AC09FD6u,
    0x6089CD06u, 0xB8188308u, 0x81F56363u, 0x8DB7693Au, 0x2F9819F1u, 0xF769A39Eu, 0x43137780u, 0xC7AE16C4u,
    0x217E62F3u, 0xE0D4E423u, 0x7AE4DD86u, 0x6637918Au, 0x5446039Du, 0xF239B091u, 0xE41DBBD7u, 0x7D4BCDFFu,
    0x31E65B75u, 0xE3700259u, 0x938288CEu, 0x001E035Au, 0xB31CA9EAu, 0x5A4352B1u, 0x88C8C98Fu, 0x295C6371u,
    0xBA22DF99u, 0x10C019EEu, 0xD1EC3810u, 0x73F3B85Du, 0xB02C5FEDu, 0x7BB6B037u, 0x7E0695D2u, 0xE79E0BB6u,
    0x47831148u, 0xF1E9ACE6u, 0x8D5AB928u, 0xC8B73808u, 0xED474844u, 0x9B665676u, 0x7810B52Fu, 0xE572F0B2u,
    0x9F041C4Du, 0x5FA728B5u, 0x77FB5257u, 0x11756D67u, 0xCE8547E0u, 0x912C0F07u, 0xA906F4F8u, 0x88AF18E8u,
  };
  uint16_t original[DISCOVERY_W * DISCOVERY_H];
  gfx565::target(g_fb);
  for (int i = 0; i < DISCOVERY_COUNT; i++) {
    gfx565::clear(0);
    assert(packed565::draw(DISCOVERY_ART[i], 0, 0, 1));
    for (int y = 0; y < DISCOVERY_H; y++)
      memcpy(original + y * DISCOVERY_W, g_fb + y * gfx565::W, DISCOVERY_W * sizeof(uint16_t));
    assert(os::crc32(original, sizeof original) == originalCrc[i]);
    gfx565::clear(0);
    assert(packed565::draw(DISCOVERY_ART[i], COVER_X, COVER_Y, 3));
    for (int y = 0; y < DISCOVERY_H * 3; y++)
      for (int x = 0; x < DISCOVERY_W * 3; x++)
        assert(g_fb[(COVER_Y + y) * gfx565::W + COVER_X + x] == original[(y / 3) * DISCOVERY_W + x / 3]);
    secondTarget(DISCOVERY_ART[i], COVER_X, COVER_Y, 3);
  }
}

// Frozen before the packing change from the baseline RLE pixels. Each of these 504 entries was
// independently matched to the original manifest SHA-256; repeated animation frames remain explicit.
static const uint32_t SCENE_ORIGINAL_CRC[] = {
    0x7B98585Bu, 0x4AD45F7Du, 0x69CE57FFu, 0x33BE1578u, 0xC9FAEB40u, 0xA4816257u, 0x094B6A10u, 0x05A30CCDu,
    0x14843C64u, 0x14843C64u, 0x848983CFu, 0x3D612921u, 0xE585B8FBu, 0xE585B8FBu, 0x5C6D1215u, 0xE585B8FBu,
    0x65B8EA43u, 0x65B8EA43u, 0x22AD6288u, 0x65B8EA43u, 0x206979BBu, 0xE421D83Fu, 0xAABD8A3Cu, 0x206979BBu,
    0x1D7F23E8u, 0x1D7F23E8u, 0x1746FA42u, 0x1D7F23E8u, 0x30837EBDu, 0x30837EBDu, 0x5140863Bu, 0x8992B371u,
    0xDB8FAB0Bu, 0x8841EF73u, 0xE959121Eu, 0x182B9D7Bu, 0xB3DE820Au, 0xB3DE820Au, 0xF34A21AFu, 0x89E88DF6u,
    0x7B98585Bu, 0xB97AB1A7u, 0xDEFF95A4u, 0xB97AB1A7u, 0x7B98585Bu, 0xEBA84FA2u, 0x1B496ABFu, 0x702B4F19u,
    0x0FFD8725u, 0x639C334Fu, 0x42D54DA1u, 0x918DA793u, 0xEEDB2AA3u, 0x778B9685u, 0x7A829343u, 0x048FB67Du,
    0xE77931ACu, 0xDC2F749Eu, 0xC14ABD4Du, 0x4AFA7BADu, 0x58CC07AEu, 0x0C1B4451u, 0x0A34CE2Bu, 0x7534D86Fu,
    0xC103E2BEu, 0xC103E2BEu, 0xFC6D0899u, 0x71E02132u, 0x7964D10Cu, 0x7964D10Cu, 0xF4E9F8A7u, 0x7964D10Cu,
    0x8C742E3Du, 0x8C742E3Du, 0xCB61A6F6u, 0x8C742E3Du, 0x206979BBu, 0xE421D83Fu, 0xAABD8A3Cu, 0x206979BBu,
    0xB82308A4u, 0xB82308A4u, 0x867F524Bu, 0xB82308A4u, 0x3484BD39u, 0x3484BD39u, 0x532E26EFu, 0x8BFC13A5u,
    0x584B9B0Eu, 0x71A0BB2Fu, 0x8836FEE5u, 0x51CC07CBu, 0x02A0E833u, 0x02A0E833u, 0xA859D850u, 0x0364F698u,
    0xE77931ACu, 0x3DD99499u, 0x7B5FA43Eu, 0x3DD99499u, 0xE77931ACu, 0xE0C2C742u, 0xF710325Du, 0xAD829B41u,
    0x931CEED2u, 0x4361EE80u, 0x79F1DE03u, 0xF3CC1FFFu, 0x84283976u, 0x453FDD42u, 0xE58103FEu, 0xABC5C151u,
    0x448B51B0u, 0x9D795E46u, 0x2D5B804Bu, 0xB2565818u, 0xCB2D62BDu, 0x5FF05043u, 0x412B39CFu, 0xAFDA45C1u,
    0x2DAEDD18u, 0x2DAEDD18u, 0x72483894u, 0xB0264C25u, 0x3E1E216Du, 0x3E1E216Du, 0xFC7055DCu, 0x3E1E216Du,
    0xD0589B76u, 0xD0589B76u, 0xC20A2A1Fu, 0xD0589B76u, 0x206979BBu, 0xE421D83Fu, 0xAABD8A3Cu, 0x206979BBu,
    0x35C9418Fu, 0x35C9418Fu, 0x1CBFE510u, 0x35C9418Fu, 0xF18E7C17u, 0xF18E7C17u, 0x5EFA4523u, 0x22758461u,
    0x40B970F5u, 0xD1C16909u, 0x44241CDCu, 0xBC5C81EEu, 0x0A943676u, 0x0A943676u, 0xEF8C84F2u, 0x55ECBD53u,
    0x448B51B0u, 0xE4F0CA1Fu, 0xD6CE27CDu, 0xE4F0CA1Fu, 0x448B51B0u, 0x5D2F5D72u, 0xDCB2F8DFu, 0x83C3D55Eu,
    0x30EE8ECEu, 0xED0FD3D0u, 0x794868DBu, 0x614D35EEu, 0xD1099D9Cu, 0x015B871Eu, 0x8D302F00u, 0x92F4F4EDu,
    0xE9A040CEu, 0xD8EC47E8u, 0xFBF64F6Au, 0xA1860DEDu, 0x4391D8E4u, 0x95882095u, 0xCF7D20F2u, 0x64C9534Cu,
    0x86BC24F1u, 0x86BC24F1u, 0x16B19B5Au, 0xAF5931B4u, 0x37B518FBu, 0x37B518FBu, 0x8E5DB215u, 0x37B518FBu,
    0xF780F2D6u, 0xF780F2D6u, 0xB0957A1Du, 0xF780F2D6u, 0xB251612Eu, 0x7619C0AAu, 0x388592A9u, 0xB251612Eu,
    0x3579FC3Au, 0x3579FC3Au, 0x3F402590u, 0x3579FC3Au, 0xA2BB6628u, 0xA2BB6628u, 0xC3789EAEu, 0x1BAAABE4u,
    0x49B7B39Eu, 0x67461967u, 0x1F3C652Eu, 0xF362A1F2u, 0x43E9BF3Bu, 0x43E9BF3Bu, 0xBA0CBF22u, 0xC0AE137Bu,
    0xE9A040CEu, 0x1C716984u, 0x2286EF88u, 0x1C716984u, 0xE9A040CEu, 0x516CA62Du, 0xEAA79940u, 0xA3F2A958u,
    0x9DC59FB0u, 0x3702BDADu, 0x57B6F45Eu, 0x2507B599u, 0x7CE33236u, 0xF5D05931u, 0x4583F753u, 0x948928A3u,
    0x75412939u, 0x4E176C0Bu, 0x5372A5D8u, 0xD8C26338u, 0xD2A7340Au, 0x3D120693u, 0xCC0284C9u, 0x145E87EEu,
    0x533BFA2Bu, 0x533BFA2Bu, 0x6E55100Cu, 0xE3D839A7u, 0xAB54710Cu, 0xAB54710Cu, 0x26D958A7u, 0xAB54710Cu,
    0x1E4C36A8u, 0x1E4C36A8u, 0x5959BE63u, 0x1E4C36A8u, 0xB251612Eu, 0x7619C0AAu, 0x388592A9u, 0xB251612Eu,
    0x9025D776u, 0x9025D776u, 0xAE798D99u, 0x9025D776u, 0xA6BCA5ACu, 0xA6BCA5ACu, 0xC1163E7Au, 0x19C40B30u,
    0xCA73839Bu, 0x9EA74D3Bu, 0x7E5389D5u, 0xBA853B42u, 0xF297D502u, 0xF297D502u, 0xE11F46DDu, 0x4A226815u,
    0x75412939u, 0x98D24CBAu, 0x8726DE12u, 0x98D24CBAu, 0x75412939u, 0x5A062ECDu, 0x06FEC1A2u, 0x7E5B7D00u,
    0x0124F647u, 0x17FF6062u, 0x6C9267FCu, 0x47460DF5u, 0x161021E3u, 0xC76412F6u, 0xDA8067EEu, 0x3BC35F8Fu,
    0xD6B34925u, 0x0F4146D3u, 0xBF6398DEu, 0x206E408Du, 0x41465119u, 0x6EF91281u, 0x871D732Du, 0xCEB01A40u,
    0xBF96C58Du, 0xBF96C58Du, 0xE0702001u, 0x221E54B0u, 0xEC2E816Du, 0xEC2E816Du, 0x2E40F5DCu, 0xEC2E816Du,
    0x426083E3u, 0x426083E3u, 0x5032328Au, 0x426083E3u, 0xB251612Eu, 0x7619C0AAu, 0x388592A9u, 0xB251612Eu,
    0x1DCF9E5Du, 0x1DCF9E5Du, 0x34B93AC2u, 0x1DCF9E5Du, 0x63B66482u, 0x63B66482u, 0xCCC25DB6u, 0xB04D9CF4u,
    0xD2816860u, 0x3EC69F1Du, 0xB2416BECu, 0x5715BD67u, 0xFAA30B47u, 0xFAA30B47u, 0xA6CA1A7Fu, 0x1CAA23DEu,
    0xD6B34925u, 0x41FB123Cu, 0x2AB75DE1u, 0x41FB123Cu, 0xD6B34925u, 0xE7EBB4FDu, 0x2D5C0B20u, 0x501A331Fu,
    0xA2D6965Bu, 0xB9915D32u, 0x6C2BD124u, 0xD5C727E4u, 0x43318509u, 0x830048AAu, 0xB2314B10u, 0x02F26A33u,
    0x61D57E9Cu, 0x509979BAu, 0x7AC8379Cu, 0x29F333BFu, 0x143C2171u, 0x06901CBAu, 0xB4EE3385u, 0xAC2A7FC5u,
    0x0EC91AA3u, 0x0EC91AA3u, 0x978FE3ACu, 0x272C0FE6u, 0x2178A07Fu, 0x2178A07Fu, 0x91DB4C35u, 0x2178A07Fu,
    0x623C9EFCu, 0x623C9EFCu, 0x6683134Bu, 0x623C9EFCu, 0x126199D8u, 0xD629385Cu, 0x98B56A5Fu, 0x126199D8u,
    0xA7F1379Eu, 0xA7F1379Eu, 0x76A44336u, 0xA7F1379Eu, 0xA207B505u, 0xA207B505u, 0x2D416176u, 0x1B1678C9u,
    0xC1C28DCCu, 0x3700E1DCu, 0x477A48F4u, 0xC10D5285u, 0xB98D5E97u, 0xB98D5E97u, 0xE622F9E3u, 0x47ECF8B8u,
    0x61D57E9Cu, 0xCE5B8BF1u, 0xB1A998DEu, 0xCE5B8BF1u, 0x61D57E9Cu, 0xA361C8DBu, 0x8C54CAA0u, 0xECD4335Cu,
    0x15B0A1E2u, 0x43FDDE92u, 0x3C96987Au, 0x94883632u, 0xF4960C64u, 0x115CFE66u, 0x97D4014Bu, 0xEBCB1F72u,
    0x1B60FCF3u, 0x2036B9C1u, 0xD24CDD2Eu, 0xB6E3B6F2u, 0xDC2C7472u, 0xF72C8351u, 0xB79197BEu, 0x859B128Au,
    0x3D1A2FE1u, 0x3D1A2FE1u, 0xEF6B68FAu, 0x8DF9EC6Du, 0x5BCD2210u, 0x5BCD2210u, 0x395FA687u, 0x5BCD2210u,
    0x8BF05A82u, 0x8BF05A82u, 0x8F4FD735u, 0x8BF05A82u, 0x126199D8u, 0xD629385Cu, 0x98B56A5Fu, 0x126199D8u,
    0xDCAB2A48u, 0xDCAB2A48u, 0xDFCF363Du, 0xDCAB2A48u, 0xA6007681u, 0xA6007681u, 0x2F2FC1A2u, 0x1978D81Du,
    0xA4525651u, 0x17B0CCA0u, 0x2615A40Fu, 0x88EAC835u, 0xEEA7DF36u, 0xEEA7DF36u, 0xBD31001Cu, 0x2B34684Eu,
    0x1B60FCF3u, 0xF3FB13EFu, 0x38255527u, 0xF3FB13EFu, 0x1B60FCF3u, 0xA80B403Bu, 0x00F450D5u, 0x6BD4DD95u,
    0x6F05238Du, 0x8554E8C5u, 0x07B20BD8u, 0x109D65C6u, 0x7831F429u, 0xC5BC5E39u, 0x08D791F6u, 0xA2D583C6u,
    0xED0E84C8u, 0x34FC8B3Eu, 0x1F536028u, 0x1BD38D60u, 0xD6F44664u, 0x3DFEC046u, 0x56C9C394u, 0xC64CD821u,
    0x842B0860u, 0x842B0860u, 0x4040D8F7u, 0x19A3995Du, 0x492BCA56u, 0x492BCA56u, 0x10C88BFCu, 0x492BCA56u,
    0x10AC1624u, 0x10AC1624u, 0x52E68204u, 0x10AC1624u, 0x126199D8u, 0xD629385Cu, 0x98B56A5Fu, 0x126199D8u,
    0xD2424B0Bu, 0xD2424B0Bu, 0xB29E3129u, 0xD2424B0Bu, 0x7A676212u, 0x7A676212u, 0x03F5226Eu, 0xA99C9A64u,
    0xE93CA58Du, 0x74B45ADBu, 0x0371FDDCu, 0x657A4E10u, 0xB30F1954u, 0xB30F1954u, 0xDBEADCBEu, 0x28203BA2u,
    0xED0E84C8u, 0xDFC40915u, 0xAF74EA74u, 0xDFC40915u, 0xED0E84C8u, 0x4B81B87Eu, 0xDB142552u, 0xFA2C9F80u,
    0x996B5BB6u, 0x7EA6CDB2u, 0x26053D00u, 0xD78057F0u, 0x788C48E4u, 0xD4441C42u, 0x41683D08u, 0xCE78AE5Du,
};
static void scenePixels() {
  static_assert(sizeof SCENE_ORIGINAL_CRC / sizeof SCENE_ORIGINAL_CRC[0] ==
                SCENE_STAGES * SCENE_TIMES * SCENE_ACTIVITIES * SCENE_FRAMES, "every scene reference");
  static uint16_t original[SCENE_W * SCENE_H];
  int i = 0;
  for (const auto& stage : SCENES)
    for (const auto& time : stage)
      for (const auto& activity : time)
        for (const packed565::Image& scene : activity) {
          gfx565::target(g_fb);
          gfx565::clear(0);
          assert(packed565::draw(scene, 0, 0, 1));
          for (int y = 0; y < SCENE_H; y++)
            memcpy(original + y * SCENE_W, g_fb + y * gfx565::W, SCENE_W * sizeof(uint16_t));
          assert(os::crc32(original, sizeof original) == SCENE_ORIGINAL_CRC[i++]);
          assert(packed565::draw(scene, 0, 0, 3));
          for (int y = 0; y < gfx565::H; y++)
            for (int x = 0; x < gfx565::W; x++)
              assert(g_fb[y * gfx565::W + x] == original[(y / 3) * SCENE_W + x / 3]);
          secondTarget(scene, 0, 0, 3);
        }
  gfx565::target(g_fb);
}

static const char* filled(const char* s) {   // the screens fill into 256 bytes: never cut there
  static char out[1024];
  const size_t n = personalize(s, NAME, PET, out, sizeof out);
  if (n >= 256) { printf("content gate: %zu bytes filled, the screens hold 255: \"%s\"\n", n, out); failures++; }
  return out;
}
// No taller than l.box.h, and every pixel textBox paints (it clips like the device's label) is on the glass.
static void fits(const Label& l, const char* where, const char* s) {
  const font::Box& b = l.box;
  int h = font::textHeight(*b.font, s, b.w, b.spacing), y = l.middle ? l.y + (b.h - h) / 2 : l.y;
  if (h > b.h) { printf("content gate: %s is %d px tall, its box %d: \"%s\"\n", where, h, b.h, s); failures++; return; }
  gfx565::clear(0);
  font::textBox(b, s, l.x, y, 0xFFFF);
  for (int py = 0; py < gfx565::H; py++)
    for (int px = 0; px < gfx565::W; px++)
      if (gfx565::fb[py * gfx565::W + px] && !gfx565::inCircle(px, py)) {
        printf("content gate: %s is cut by the round edge at (%d, %d): \"%s\"\n", where, px, py, s);
        failures++;
        return;
      }
}
// A story up to its choice, an ending or a discovery, filled and joined with a space as Game::openPages does, in at
// most READ_BYTES and READ_SCREENS screens of PAGE, at the widest names and at short ones. The flow fills every screen
// but the last until the next word no longer fits, and pageBreaks gives the last one MIN_LAST_WORDS words when the
// screen before can spare them, so only the last screen can be short: it may not be.
static int words(const char* s) {   // pageBreaks leaves single spaces between words
  int n = *s != 0;
  for (; *s; s++) n += *s == ' ';
  return n;
}
static void stream(const char* where, const char* const* pages, int count, const char* name, const char* pet) {
  static char buf[4 * READ_BYTES];
  size_t n = 0;
  for (int i = 0; i < count && n + 1 < sizeof buf; i++) {
    if (i) buf[n++] = ' ';
    n += personalize(pages[i], name, pet, buf + n, sizeof buf - n);
  }
  if (n >= READ_BYTES) { printf("content gate: %s fills %zu bytes, the reader holds %d\n", where, n, READ_BYTES - 1); failures++; return; }
  const char* screens[READ_SCREENS];
  const int k = font::pageBreaks(PAGE.box, buf, screens, READ_SCREENS, MIN_LAST_WORDS);
  if (k > READ_SCREENS) { printf("content gate: %s takes %d screens, the reader holds %d\n", where, k, READ_SCREENS); failures++; return; }
  for (int i = 0; i < k; i++) fits(PAGE, where, screens[i]);
  if (words(screens[k - 1]) < MIN_LAST_WORDS) {
    printf("content gate: %s ends on a %d-word screen: \"%s\"\n", where, words(screens[k - 1]), screens[k - 1]);
    failures++;
  }
}
static void stream(const char* where, const char* const* pages, int count) {
  char shortNames[80];
  snprintf(shortNames, sizeof shortNames, "%s (Sam / Pip)", where);
  stream(where, pages, count, NAME, PET);
  stream(shortNames, pages, count, "Sam", "Pip");
}
static void storyGate() {
  char where[64], text[128];
  for (const Story& st : STORIES) {
    snprintf(text, sizeof text, "Day %d: %s", st.unlockDay, st.title);
    fits(STORY_BUTTON, st.id, text);
    snprintf(text, sizeof text, "%s *", st.title);
    fits(STORY_BUTTON, st.id, text);
    fits(SUBTITLE, st.id, st.subtitle);
    snprintf(where, sizeof where, "%s story", st.id);
    stream(where, st.pages, STORY_PAGES);
    snprintf(where, sizeof where, "%s prompt", st.id);
    fits(PROMPT, where, filled(st.prompt));
    for (const StoryChoice& c : st.choices) {
      fits(CHOICE_BUTTON, st.id, c.label);
      snprintf(where, sizeof where, "%s '%s'", st.id, c.label);
      stream(where, c.ending, ENDING_PAGES);
    }
  }
}
static void discoveryGate() {
  char where[64];
  for (const Topic& t : TOPICS) { fits(HEADER, t.id, t.name); fits(rowText(t.name), t.id, t.name); }
  for (const Discovery& d : DISCOVERIES) {
    fits(coverTitle(d.title), d.id, d.title);
    fits(rowText(d.title), d.id, d.title);
    stream(d.id, d.pages, 2);
    snprintf(where, sizeof where, "%s wonder", d.id);
    fits(WONDER, where, filled(d.wonder));
    fits(SOURCE_NAME, d.id, d.sourceName);
    fits(SOURCE_URL, d.id, d.sourceUrl);
  }
}
static void dailyGate() {
  char text[64];
  for (const Adventure& a : ADVENTURES) {
    fits(HEADER, a.title, a.title);
    fits(ADVENTURE, a.title, filled(a.description));
    fits(HEADER, a.title, a.word);
    fits(MEANING, a.word, filled(a.meaning));
  }
  for (uint8_t i = 0; i < TRICK_COUNT; i++) {
    const Trick& t = TRICKS[i];
    fits(HEADER, t.id, t.name);
    snprintf(text, sizeof text, "%s   Day %d", t.name, trickUnlockDay(i));
    fits(ROW_LABEL, t.id, text);
    snprintf(text, sizeof text, "%s   3/3", t.name);
    fits(ROW_LABEL, t.id, text);
  }
  Label album = STICKER;   // the lowest of the album's rows
  album.y = (int16_t)(album.y + (STICKER_ROWS - 1) * STICKER_STEP);
  for (const char* s : STICKERS) fits(album, s, s);
  fits(album, "album", "A little surprise awaits");
  for (int bit = 0; bit < 6; bit++) {   // Today's activities, once done too
    snprintf(text, sizeof text, "* %s", ACTIVITIES[bit]);
    fits(TODAY_BUTTON, ACTIVITIES[bit], text);
  }
}
// The pup's own lines and the screens' status text, at the widest names and the biggest counts a save allows.
static void homeGate() {
  char text[160];
  const unsigned big = MAX_COUNT, stars = NUM_STORIES * 3;
  for (int i = 0; i < SAY_COUNT; i++) { snprintf(text, sizeof text, "SAY[%d]", i); fits(BUBBLE, text, filled(SAY[i])); }
  for (const char* stage : STAGES) {
    snprintf(text, sizeof text, "Day %u \xC2\xB7 %s", big, stage);             // Home's mood (a lowercase first letter)
    fits(HOME_MOOD, stage, text);
    snprintf(text, sizeof text, "Day %u \xC2\xB7 %s \xC2\xB7 %u stars", big, stage, stars);   // World
    fits(WORLD_LINE, stage, text);
    snprintf(text, sizeof text, "%s - Cuddlebug", stage);                        // the scrapbook's badge
    fits(BOOK_BADGE, stage, text);
  }
  fits(HOME_MOOD, "asleep", "dreaming of stories");
  static const char* const ACTION_LABELS[4][2] = {{"Feed", nullptr}, {"Play", nullptr}, {"Read", nullptr}, {"More", "Wake"}};
  for (int i = 0; i < 4; i++)
    for (const char* s : ACTION_LABELS[i]) if (s) fits(actionLabel(ACTIONS[i], &FONT20), "action", s);
  fits(WORLD_NAMES, "pet alone", PET);   // World shows "{pet} & {name}" only when it fits, else the pup's name
  snprintf(text, sizeof text, "{name} & {pet}\nDay %u together\n%u friendship\n%u story stars", big, big, stars);
  fits(BOOK_LINES, "scrapbook", filled(text));
}
// The words the new screens compose around the content: titles, button labels, counters, World's tiles.
static Label inTile(const Label& l, const Box& tile) {
  return {l.box, (int16_t)(tile.x * 3 + l.x), (int16_t)(tile.y * 3 + l.y), l.middle};
}
static void screensGate() {
  static const char* const TITLES[] = {"Our bookshelf", "Story time", "The story continues", "What shall we do?",
                                       "Little discoveries", "Our little notebook", "So much to explore",
                                       "Where we found it", "Our sticker album", "Our scrapbook",
                                       "Little paws, big ideas"};
  for (const char* t : TITLES) fits(HEADER, "title", t);
  static const char* const PAIRS[] = {"Discoveries", "Notebook", "Topics", "Today's three"};
  for (const char* t : PAIRS) fits(PAIR_LABEL, "pair", t);
  static const char* const NAVS[] = {"Previous", "Next", "Choose", "The end", "Source", "Keep", "Our stickers", "Rename pup"};
  for (const char* t : NAVS) { fits(buttonLabel(PREV, &FONT20), "prev", t); fits(buttonLabel(NEXT, &FONT20), "next", t); }
  static const char* const WIDES[] = {"Let's find out", "Back to our book", "Back to today", "My turn", "Peek again"};
  for (const char* t : WIDES) fits(buttonLabel(WIDE, &FONT20), "wide", t);
  fits(TODAY_BUTTON, "word", "A lovely word");
  fits(WONDER_TITLE, "wonder", "I wonder...");
  fits(NOTEBOOK_EMPTY, "notebook", "A place for all the things we find together.");
  static_assert(READ_SCREENS <= MAX_PAGES && NUM_DISCOVERIES / 2 <= MAX_PAGES, "the counters' widest");
  char s[16];
  for (int count = 1; count <= MAX_PAGES; count++)
    for (int page = 0; page < count; page++) {
      pageNumber(s, page, count);
      if (font::textWidth(*PAGE_NUMBER.box.font, s) > PAGE_NUMBER.box.w) { printf("content gate: counter \"%s\" is too wide\n", s); failures++; }
    }
  pageNumber(s, MAX_PAGES - 1, MAX_PAGES);
  fits(PAGE_NUMBER, "counter", s);
  const Box tiles[] = {WORLD_TRICKS, WORLD_NAP, WORLD_BOOK, WORLD_TODAY};
  const char* const tileTitles[4][2] = {{"Learn tricks", nullptr}, {"Cozy nap", "Wake up"}, {"Our scrapbook", nullptr},
                                        {"Today's adventure", nullptr}};
  const char* const tileDetails[4][2] = {{"6 of 6 mastered", nullptr}, {"A lovely place to pause", nullptr},
                                         {"Growing up, page by page", nullptr}, {"Something to discover", "Sticker earned!"}};
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 2; j++) {
      if (tileTitles[i][j]) fits(inTile(TILE_TITLE, tiles[i]), "tile", tileTitles[i][j]);
      if (tileDetails[i][j]) fits(inTile(TILE_DETAIL, tiles[i]), "tile", tileDetails[i][j]);
    }
}
static void pixelGate() {
  const font::Font* F[4] = {&FONT16, &FONT20, &FONT24, &FONT28};
  for (const font::Font* f : F)
    for (char c = 'A'; c <= 'z'; c++) {   // the names above are the widest: W is the widest letter, letters never kern
      const char s[2] = {c, 0};
      int g = font::glyphIndex(*f, (uint32_t)c);
      assert(!isalpha(c) || (font::textWidth(*f, s) <= font::textWidth(*f, "W") && !f->kernLeft[g] && !f->kernRight[g]));
    }
  gfx565::target(g_fb);
  storyGate();
  discoveryGate();
  dailyGate();
  homeGate();
  screensGate();
  if (failures) printf("content gate: %d strings do not fit (widest names: {name} %s, {pet} %s)\n", failures, NAME, PET);
  assert(failures == 0);
}

int main() {
  personalizing();
  art();
  discoveryPixels();
  scenePixels();
  pixelGate();
  // The rules and the content count the same things (and share one namespace without clashing).
  static_assert(NUM_STORIES == STORY_COUNT && NUM_DISCOVERIES == DISCOVERY_COUNT && NUM_TRICKS == TRICK_COUNT &&
                NUM_STICKERS == STICKER_COUNT, "pet.h and the content agree");
  assert(STORY_COUNT == 7 && TOPIC_COUNT == 12 && ADVENTURE_COUNT == 7 && TRICK_COUNT == 6 && STICKER_COUNT == 12);
  printf("biscuit content: all checks passed\n");
  return 0;
}
