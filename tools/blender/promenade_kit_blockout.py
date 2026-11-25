# -*- coding: utf-8 -*-
"""
EBBSHIFT - Promenade sahil cephesi kiti, gri kutu / ilk gecis modulleri.

Ne yapar
--------
"PRM_Kit_Blockout" adinda YENI bir sahne acar (mevcut sahneye dokunmaz) ve
5 moduler parcayi 1 Blender birimi = 1 cm olacak sekilde kurar:

    SM_PRM_Wall_400x400_A       dolu duvar            400 x 25 x 400 cm
    SM_PRM_WallDoor_400x400_A   kapili duvar          kapi boslugu 150 x 250 cm
    SM_PRM_Floor_400x400_A      zemin plagi           400 x 400 x 25 cm (ust yuz Z=0)
    SM_PRM_Parapet_400_A        dolu deniz duvari     400 x 25 x 110 cm (alcak siper)
    SM_PRM_Railing_400_A        acik korkuluk         400 x 10 x 110 cm (siper DEGIL)

Her parcaya UCX_ basit carpisma kutulari, 400 cm = 1 UV birimi kutu izdusumu
UV (2048 px tiling dokuda 512 px/m) ve beyaz "Col" vertex rengi eklenir.
Ayrica palet renkleriyle boyanmis 3 akslik, 2 katli bir ornek cephe ve
192 cm'lik olcek mankeni (oyuncu kapsulu) kurulur (REF_ onekli nesneler export edilmez).

Calistirma
----------
    blender -b -P tools/blender/promenade_kit_blockout.py -- --out <klasor>
    blender -b -P tools/blender/promenade_kit_blockout.py -- --no-export --save-blend kit.blend
    python3 tools/blender/promenade_kit_blockout.py --selftest   (Blender'siz geometri testi)

Blender'in Scripting sekmesinden ya da Blender MCP execute_blender_code ile de
calistirilabilir; o durumda varsayilan cikti klasoru .blend dosyasinin yanindaki
"export/promenade_kit" olur (.blend kaydedilmemisse gecici klasor).

Sozlesme (fps-art-pipeline)
---------------------------
Birim: Metric, Unit Scale 0.01, uzunluk cm. FBX: -Y Forward, Z Up, olcek 1.0,
FBX_SCALE_NONE, Smoothing=Face, tangent space acik. Pivot: taban, min-X kose;
duvarin dis (sokak) yuzu Y=0 duzleminde, kalinlik +Y yonunde (bina ici).
Zemin plaginin ust yuzu Z=0.

Durum: Blender 4.x / 5.x API'sine gore yazildi. Geometri kismi saf Python
ile test edildi (--selftest). bpy / FBX kismi yazildigi oturumda
CALISTIRILMADI; ilk calistirmada UE5 importunda olcek mankeniyle dogrulayin.
"""

import os
import sys

try:
    import bpy  # type: ignore
    import bmesh  # type: ignore
except ImportError:  # Blender disinda: yalnizca --selftest calisir
    bpy = None
    bmesh = None


# --------------------------------------------------------------------------
# Sabitler (cm)
# --------------------------------------------------------------------------

GRID = 50            # en kucuk izgara adimi
BAY = 400            # cephe aksi genisligi
STOREY = 400         # kat yuksekligi (doseme ustunden doseme ustune)
WALL_T = 25          # duvar kalinligi
FLOOR_T = 25         # doseme kalinligi
DOOR_W = 150         # fps-level-design: kapi 120-150
DOOR_H = 250         # fps-level-design: kapi 220-250
LOW_COVER_H = 110    # fps-level-design: alcak siper 100-110
MANNEQUIN_RADIUS = 40    # NFCharacter.cpp InitCapsuleSize(40, 96)
MANNEQUIN_HEIGHT = 192   # 2 x yari yukseklik
MANNEQUIN_EYE = 160      # 96 + BaseEyeHeight 64
MANNEQUIN_CROUCH = 120   # 2 x CrouchedHalfHeight 60
UV_TILE = 400.0      # 400 cm = 1 UV birimi -> 2048 px dokuda 512 px/m

SCENE_NAME = "PRM_Kit_Blockout"
COL_EXPORT = "PRM_KIT_EXPORT"
COL_COLLISION = "PRM_KIT_COLLISION"
COL_ASSEMBLY = "PRM_KIT_ASSEMBLY"
ASM_OFFSET = (1000.0, 0.0, 0.0)   # ornek cephenin kaynak parcalardan uzakligi

# Stil rehberi paleti (docs/studio/art/style-guide.md, bolum 2). sRGB hex.
PALETTE = {
    "M_PRM_Blockout_Mint": "#A8CDB8",
    "M_PRM_Blockout_Pink": "#E3B1B4",
    "M_PRM_Blockout_Cream": "#CFC5AA",
    "M_PRM_Blockout_SkyBlue": "#A9C3CF",
    "M_PRM_Blockout_Concrete": "#9A9A92",
    "M_PRM_Blockout_Brass": "#A8873F",
    "M_PRM_Blockout_Deck": "#7A6A58",
    "M_REF_Mannequin": "#E63B2E",
}


# --------------------------------------------------------------------------
# Saf Python geometri (bpy gerektirmez)
# --------------------------------------------------------------------------

def cells_to_mesh(xs, ys, zs, filled):
    """Eksenlere hizali hucre izgarasindan kapali (manifold) dortgen ag uretir.

    xs/ys/zs: artan koordinat listeleri. filled: dolu hucrelerin (i, j, k)
    kumesi. Yalnizca komsusu bos olan yuzler uretilir; ic yuz kalmaz.
    Donus: (verts, faces) - faces disaridan bakildiginda saat yonunun tersi.
    """
    filled = set(filled)
    index = {}
    verts = []

    def vid(p):
        if p not in index:
            index[p] = len(verts)
            verts.append(p)
        return index[p]

    faces = []
    for (i, j, k) in sorted(filled):
        x0, x1 = xs[i], xs[i + 1]
        y0, y1 = ys[j], ys[j + 1]
        z0, z1 = zs[k], zs[k + 1]
        quads = (
            ((i + 1, j, k), ((x1, y0, z0), (x1, y1, z0), (x1, y1, z1), (x1, y0, z1))),  # +X
            ((i - 1, j, k), ((x0, y1, z0), (x0, y0, z0), (x0, y0, z1), (x0, y1, z1))),  # -X
            ((i, j + 1, k), ((x1, y1, z0), (x0, y1, z0), (x0, y1, z1), (x1, y1, z1))),  # +Y
            ((i, j - 1, k), ((x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1))),  # -Y
            ((i, j, k + 1), ((x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1))),  # +Z
            ((i, j, k - 1), ((x0, y1, z0), (x1, y1, z0), (x1, y0, z0), (x0, y0, z0))),  # -Z
        )
        for neighbour, quad in quads:
            if neighbour not in filled:
                faces.append(tuple(vid(tuple(float(c) for c in p)) for p in quad))
    return verts, faces


def all_cells(xs, ys, zs):
    return {(i, j, k)
            for i in range(len(xs) - 1)
            for j in range(len(ys) - 1)
            for k in range(len(zs) - 1)}


def box(x0, x1, y0, y1, z0, z1):
    xs, ys, zs = [x0, x1], [y0, y1], [z0, z1]
    return cells_to_mesh(xs, ys, zs, all_cells(xs, ys, zs))


def part_definitions():
    """Her parca: ad, (xs, ys, zs, filled), UCX kutulari, varsayilan materyal, not."""
    parts = []

    # 1) Dolu duvar
    xs, ys, zs = [0, BAY], [0, WALL_T], [0, STOREY]
    parts.append({
        "name": "SM_PRM_Wall_400x400_A",
        "grid": (xs, ys, zs, all_cells(xs, ys, zs)),
        "ucx": [(0, BAY, 0, WALL_T, 0, STOREY)],
        "material": "M_PRM_Blockout_Mint",
        "note": "tam siper",
    })

    # 2) Kapili duvar: kapi boslugu ortada, 150 x 250
    side = (BAY - DOOR_W) / 2.0
    xs, ys, zs = [0, side, side + DOOR_W, BAY], [0, WALL_T], [0, DOOR_H, STOREY]
    filled = all_cells(xs, ys, zs) - {(1, 0, 0)}
    parts.append({
        "name": "SM_PRM_WallDoor_400x400_A",
        "grid": (xs, ys, zs, filled),
        "ucx": [
            (0, side, 0, WALL_T, 0, STOREY),
            (side + DOOR_W, BAY, 0, WALL_T, 0, STOREY),
            (side, side + DOOR_W, 0, WALL_T, DOOR_H, STOREY),
        ],
        "material": "M_PRM_Blockout_Pink",
        "note": "kapi %dx%d" % (DOOR_W, DOOR_H),
    })

    # 3) Zemin plagi: ust yuz Z=0
    xs, ys, zs = [0, BAY], [0, BAY], [-FLOOR_T, 0]
    parts.append({
        "name": "SM_PRM_Floor_400x400_A",
        "grid": (xs, ys, zs, all_cells(xs, ys, zs)),
        "ucx": [(0, BAY, 0, BAY, -FLOOR_T, 0)],
        "material": "M_PRM_Blockout_Concrete",
        "note": "yurunur yuzey Z=0",
    })

    # 4) Dolu deniz duvari (alcak siper)
    xs, ys, zs = [0, BAY], [0, WALL_T], [0, LOW_COVER_H]
    parts.append({
        "name": "SM_PRM_Parapet_400_A",
        "grid": (xs, ys, zs, all_cells(xs, ys, zs)),
        "ucx": [(0, BAY, 0, WALL_T, 0, LOW_COVER_H)],
        "material": "M_PRM_Blockout_Cream",
        "note": "alcak siper %d" % LOW_COVER_H,
    })

    # 5) Acik korkuluk: 4 dikme + ust ve orta kusak. Mermi gecirir; siper degil.
    post, rail_t = 10, 10
    xs = [0, post, 130, 130 + post, 260, 260 + post, BAY - post, BAY]
    ys = [0, rail_t]
    zs = [0, 50, 58, LOW_COVER_H - 10, LOW_COVER_H]
    filled = set()
    for i in range(len(xs) - 1):
        for k in range(len(zs) - 1):
            is_post = (i % 2 == 0)
            is_rail = k in (1, 3)
            if is_post or is_rail:
                filled.add((i, 0, k))
    parts.append({
        "name": "SM_PRM_Railing_400_A",
        "grid": (xs, ys, zs, filled),
        # Oyuncuyu durduran tek kutu. Mermi gecisi icin UE'de carpisma
        # kanali ayari gerekir (gameplay-developer ile kararlastirilacak).
        "ucx": [(0, BAY, 0, rail_t, 0, LOW_COVER_H)],
        "material": "M_PRM_Blockout_Brass",
        "note": "siper degil",
    })
    return parts


def mesh_stats(verts, faces):
    """Sinir kutusu, hacim ve manifold denetimi (saf Python)."""
    lo = tuple(min(v[a] for v in verts) for a in range(3))
    hi = tuple(max(v[a] for v in verts) for a in range(3))
    directed = {}
    for f in faces:
        for a, b in zip(f, f[1:] + f[:1]):
            directed[(a, b)] = directed.get((a, b), 0) + 1
    manifold = all(n == 1 and directed.get((b, a), 0) == 1
                   for (a, b), n in directed.items())
    volume = 0.0
    for f in faces:
        p0 = verts[f[0]]
        for a, b in zip(f[1:], f[2:]):
            p1, p2 = verts[a], verts[b]
            volume += (
                p0[0] * (p1[1] * p2[2] - p1[2] * p2[1])
                - p0[1] * (p1[0] * p2[2] - p1[2] * p2[0])
                + p0[2] * (p1[0] * p2[1] - p1[1] * p2[0])
            ) / 6.0
    return {"min": lo, "max": hi, "manifold": manifold, "volume": volume,
            "quads": len(faces), "tris": 2 * len(faces), "verts": len(verts)}


def selftest():
    ok = True
    print("%-28s %5s %5s %-9s %-26s %s" % ("parca", "tris", "vert", "manifold", "boyut (cm)", "hacim (cm3)"))
    for part in part_definitions():
        xs, ys, zs, filled = part["grid"]
        verts, faces = cells_to_mesh(xs, ys, zs, filled)
        st = mesh_stats(verts, faces)
        expected = sum((xs[i + 1] - xs[i]) * (ys[j + 1] - ys[j]) * (zs[k + 1] - zs[k])
                       for (i, j, k) in filled)
        size = tuple(st["max"][a] - st["min"][a] for a in range(3))
        good = st["manifold"] and abs(st["volume"] - expected) < 1e-6
        on_grid = st["min"][0] == 0 and st["min"][1] == 0 and size[0] % GRID == 0
        ucx_ok = True
        for b in part["ucx"]:
            ucx_ok = ucx_ok and mesh_stats(*box(*b))["manifold"]
        ok = ok and good and on_grid and ucx_ok
        print("%-28s %5d %5d %-9s %-26s %.0f%s" % (
            part["name"], st["tris"], st["verts"], st["manifold"],
            "%g x %g x %g" % size, st["volume"],
            "" if (good and on_grid and ucx_ok) else "   <-- HATA"))
    print("SELFTEST", "GECTI" if ok else "KALDI")
    return ok


# --------------------------------------------------------------------------
# Blender tarafi
# --------------------------------------------------------------------------

def hex_to_linear_rgba(hex_code):
    """sRGB hex -> Blender'in bekledigi lineer RGBA."""
    def to_linear(c):
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    h = hex_code.lstrip("#")
    r, g, b = (int(h[i:i + 2], 16) / 255.0 for i in (0, 2, 4))
    return (to_linear(r), to_linear(g), to_linear(b), 1.0)


def get_material(name):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    rgba = hex_to_linear_rgba(PALETTE[name])
    mat.diffuse_color = rgba  # yalnizca Solid viewport
    mat.use_nodes = True
    # Dugum adlari yerellestirilebilir: tipe gore bul.
    bsdf = next((n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED"), None)
    if bsdf is not None:
        for socket in bsdf.inputs:
            if socket.identifier == "Base Color":
                socket.default_value = rgba
            elif socket.identifier == "Roughness":
                socket.default_value = 0.8
    return mat


def reset_collection(scene, name):
    """Betik yeniden calistirilabilsin diye ayni adli koleksiyonu temizler."""
    old = bpy.data.collections.get(name)
    if old is not None:
        for obj in list(old.objects):
            data = obj.data
            bpy.data.objects.remove(obj, do_unlink=True)
            if data is not None and data.users == 0 and isinstance(data, bpy.types.Mesh):
                bpy.data.meshes.remove(data)
        bpy.data.collections.remove(old)
    col = bpy.data.collections.new(name)
    scene.collection.children.link(col)
    return col


def build_mesh(name, verts, faces, material=None):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()

    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    uv_layer = bm.loops.layers.uv.verify()
    for face in bm.faces:
        n = face.normal
        ax, ay, az = abs(n.x), abs(n.y), abs(n.z)
        for loop in face.loops:
            co = loop.vert.co
            if ax >= ay and ax >= az:
                uv = (co.y, co.z)
            elif ay >= az:
                uv = (co.x, co.z)
            else:
                uv = (co.x, co.y)
            loop[uv_layer].uv = (uv[0] / UV_TILE, uv[1] / UV_TILE)
    bm.to_mesh(mesh)
    bm.free()

    # Vertex paint (islaklik / kir maskesi) icin beyaz renk katmani.
    if hasattr(mesh, "color_attributes"):
        attr = mesh.color_attributes.new(name="Col", type="BYTE_COLOR", domain="CORNER")
        for item in attr.data:
            item.color = (1.0, 1.0, 1.0, 1.0)

    if material is not None:
        mesh.materials.append(material)
    return mesh


def setup_scene():
    scene = bpy.data.scenes.get(SCENE_NAME) or bpy.data.scenes.new(SCENE_NAME)
    units = scene.unit_settings
    units.system = "METRIC"
    units.scale_length = 0.01       # 1 Blender birimi = 1 cm = 1 uu
    units.length_unit = "CENTIMETERS"
    window = bpy.context.window
    if window is not None:          # arka plan modunda pencere yok
        window.scene = scene
        for area in window.screen.areas:
            if area.type == "VIEW_3D":
                for space in area.spaces:
                    if space.type == "VIEW_3D":
                        space.clip_start = 1.0
                        space.clip_end = 100000.0
    return scene


def build_kit(scene):
    col_export = reset_collection(scene, COL_EXPORT)
    col_collision = reset_collection(scene, COL_COLLISION)
    col_assembly = reset_collection(scene, COL_ASSEMBLY)
    materials = {name: get_material(name) for name in PALETTE}

    assets = {}
    print("%-28s %5s  %-22s %s" % ("parca", "tris", "boyut (cm)", "not"))
    for part in part_definitions():
        xs, ys, zs, filled = part["grid"]
        verts, faces = cells_to_mesh(xs, ys, zs, filled)
        mesh = build_mesh(part["name"], verts, faces, materials[part["material"]])
        obj = bpy.data.objects.new(part["name"], mesh)
        col_export.objects.link(obj)

        ucx_objs = []
        for n, b in enumerate(part["ucx"], start=1):
            ucx_name = "UCX_%s_%02d" % (part["name"], n)
            ucx_mesh = build_mesh(ucx_name, *box(*b))
            ucx = bpy.data.objects.new(ucx_name, ucx_mesh)
            ucx.display_type = "WIRE"
            ucx.hide_render = True
            col_collision.objects.link(ucx)
            ucx_objs.append(ucx)

        assets[part["name"]] = {"object": obj, "ucx": ucx_objs}
        st = mesh_stats(verts, faces)
        size = tuple(st["max"][a] - st["min"][a] for a in range(3))
        print("%-28s %5d  %-22s %s" % (part["name"], st["tris"], "%g x %g x %g" % size, part["note"]))

    # Kaynak parcalar export icin orijinde ust uste durur; ornek cephe
    # ASM_OFFSET kadar yana kurulur ki ikisi cakismasin.
    build_assembly(col_assembly, assets, materials)
    col_export.hide_render = True
    col_collision.hide_render = True
    return assets


def place_instance(collection, source, name, location, material=None):
    """Ayni mesh verisini paylasan ornek; renk nesne duzeyinde atanir."""
    inst = bpy.data.objects.new(name, source.data)
    inst.location = tuple(p + o for p, o in zip(location, ASM_OFFSET))
    collection.objects.link(inst)
    if material is not None and len(inst.material_slots) > 0:
        inst.material_slots[0].link = "OBJECT"
        inst.material_slots[0].material = material
    return inst


def build_assembly(collection, assets, materials):
    """3 aks x 2 kat ornek cephe + promenade dosemesi + deniz kenari + manken."""
    wall = assets["SM_PRM_Wall_400x400_A"]["object"]
    door = assets["SM_PRM_WallDoor_400x400_A"]["object"]
    floor = assets["SM_PRM_Floor_400x400_A"]["object"]
    parapet = assets["SM_PRM_Parapet_400_A"]["object"]
    railing = assets["SM_PRM_Railing_400_A"]["object"]

    ground = [(wall, "M_PRM_Blockout_Mint"), (door, "M_PRM_Blockout_Pink"), (wall, "M_PRM_Blockout_Cream")]
    upper = [(wall, "M_PRM_Blockout_Mint"), (wall, "M_PRM_Blockout_Pink"), (wall, "M_PRM_Blockout_SkyBlue")]
    for i, (src, mat) in enumerate(ground):
        place_instance(collection, src, "ASM_Ground_%d" % i, (i * BAY, 0, 0), materials[mat])
    for i, (src, mat) in enumerate(upper):
        place_instance(collection, src, "ASM_Upper_%d" % i, (i * BAY, 0, STOREY), materials[mat])

    # Promenade dosemesi cephenin onunde (-Y = deniz tarafi)
    for i in range(3):
        place_instance(collection, floor, "ASM_Deck_%d" % i, (i * BAY, -BAY, 0),
                       materials["M_PRM_Blockout_Concrete"])
    # Deniz kenari: iki dolu siper, ortada acik korkuluk
    place_instance(collection, parapet, "ASM_Edge_0", (0, -BAY, 0), materials["M_PRM_Blockout_Cream"])
    place_instance(collection, railing, "ASM_Edge_1", (BAY, -BAY, 0), materials["M_PRM_Blockout_Brass"])
    place_instance(collection, parapet, "ASM_Edge_2", (2 * BAY, -BAY, 0), materials["M_PRM_Blockout_Cream"])

    # Olcek mankeni: Source/NewFPS/Private/NFCharacter.cpp'deki kapsul
    # (yaricap 40, yari yukseklik 96; goz 160; comelmis ust nokta 120).
    # Kod degisirse MANNEQUIN_* sabitlerini guncelleyin.
    r = MANNEQUIN_RADIUS
    mx = ASM_OFFSET[0] + BAY + BAY / 2.0 - r
    my = ASM_OFFSET[1] - 150 - r
    refs = (
        ("REF_Mannequin_%d" % MANNEQUIN_HEIGHT,
         box(mx, mx + 2 * r, my, my + 2 * r, 0, MANNEQUIN_HEIGHT), "M_REF_Mannequin"),
        ("REF_EyeHeight_%d" % MANNEQUIN_EYE,
         box(mx - 6, mx + 2 * r + 6, my - 6, my + 2 * r + 6, MANNEQUIN_EYE - 1, MANNEQUIN_EYE + 1),
         "M_PRM_Blockout_Deck"),
        ("REF_CrouchTop_%d" % MANNEQUIN_CROUCH,
         box(mx - 6, mx + 2 * r + 6, my - 6, my + 2 * r + 6, MANNEQUIN_CROUCH - 1, MANNEQUIN_CROUCH + 1),
         "M_PRM_Blockout_Deck"),
    )
    for name, (verts, faces), mat in refs:
        ref = bpy.data.objects.new(name, build_mesh(name, verts, faces, material=materials[mat]))
        collection.objects.link(ref)


def fbx_kwargs(filepath):
    wanted = {
        "filepath": filepath,
        "use_selection": True,
        "object_types": {"MESH"},
        "global_scale": 1.0,
        "apply_unit_scale": True,
        "apply_scale_options": "FBX_SCALE_NONE",
        "axis_forward": "-Y",
        "axis_up": "Z",
        "bake_space_transform": False,
        "use_mesh_modifiers": True,
        "mesh_smooth_type": "FACE",
        "use_tspace": True,
        "use_triangles": False,
        "colors_type": "SRGB",
        "use_custom_props": False,
        "add_leaf_bones": False,
        "bake_anim": False,
        "path_mode": "AUTO",
        "embed_textures": False,
    }
    # Surumler arasi fark: yalnizca bu Blender'in tanidigi parametreleri gonder.
    known = set(bpy.ops.export_scene.fbx.get_rna_type().properties.keys())
    return {k: v for k, v in wanted.items() if k in known}


def is_identity(obj, eps=1e-6):
    m = obj.matrix_basis
    return all(abs(m[r][c] - (1.0 if r == c else 0.0)) < eps for r in range(4) for c in range(4))


def export_assets(scene, assets, out_dir):
    if not hasattr(bpy.ops.export_scene, "fbx"):
        raise RuntimeError("FBX export eklentisi etkin degil (Preferences > Add-ons > FBX format).")
    os.makedirs(out_dir, exist_ok=True)
    view_layer = scene.view_layers[0]
    written = []
    for name, entry in assets.items():
        objs = [entry["object"]] + entry["ucx"]
        for obj in objs:
            if not is_identity(obj):
                raise RuntimeError("%s orijinde ve uygulanmis transformla olmali." % obj.name)
        for obj in view_layer.objects:
            obj.select_set(False)
        for obj in objs:
            obj.hide_set(False, view_layer=view_layer)
            obj.select_set(True, view_layer=view_layer)
        view_layer.objects.active = entry["object"]
        path = os.path.join(out_dir, name + ".fbx")
        with bpy.context.temp_override(scene=scene, view_layer=view_layer,
                                       selected_objects=objs,
                                       active_object=entry["object"],
                                       object=entry["object"]):
            bpy.ops.export_scene.fbx(**fbx_kwargs(path))
        written.append(path)
        print("FBX:", path)
    return written


def default_out_dir():
    try:
        base = os.path.dirname(os.path.abspath(__file__))
    except NameError:  # Blender metin duzenleyicisi / MCP exec
        base = bpy.path.abspath("//") if bpy.data.filepath else bpy.app.tempdir
    return os.path.join(base, "export", "promenade_kit")


def parse_args(argv):
    args = {"out": None, "export": True, "save_blend": None}
    it = iter(argv)
    for a in it:
        if a == "--out":
            args["out"] = next(it)
        elif a == "--no-export":
            args["export"] = False
        elif a == "--save-blend":
            args["save_blend"] = next(it)
    return args


def main(argv=()):
    args = parse_args(argv)
    scene = setup_scene()
    assets = build_kit(scene)
    if args["export"]:
        export_assets(scene, assets, args["out"] or default_out_dir())
    if args["save_blend"]:
        bpy.ops.wm.save_as_mainfile(filepath=os.path.abspath(args["save_blend"]))
    print("Tamam: %d parca, sahne '%s'." % (len(assets), scene.name))
    return assets


if __name__ == "__main__":
    cli = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
    if bpy is None:
        if "--selftest" in cli:
            sys.exit(0 if selftest() else 1)
        sys.exit("Bu betik Blender icinde calisir:  blender -b -P %s -- --out <klasor>\n"
                 "Blender'siz geometri testi icin:  python3 %s --selftest"
                 % (sys.argv[0], sys.argv[0]))
    main([a for a in cli if a != "--selftest"])
