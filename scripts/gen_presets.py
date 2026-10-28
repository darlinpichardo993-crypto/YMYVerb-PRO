#!/usr/bin/env python3
"""Generates Source/Presets/factory_presets.json. Values are STARTING POINTS tuned by design,
not replicas of any artist's real chain. Division index: 0=1/64 1=1/32 2=1/16 3=1/8 4=1/4 5=1/2 6=1/1"""
import json, os
BASE = dict(algo=5, room=.45, decay=2.8, predelay=32, early=.55, diffusion=.75, density=.7, mod=.25, warmth=.6, damping=.5,
            lowcut=130, highcut=8500, lowdecay=.4, highdecay=.45, tone=.55, width=.85, balance=0, erwidth=.7, spread=.8,
            dry=1.0, wet=.28, in_=0, out=0, ret=0, duckon=False, duckthr=-30, duckdepth=10, duckatt=10, duckrel=300,
            makeup=True, sync=True, bpm=130, div=1, triplet=False, dotted=False, mode=0)
P = []
def add(cat, name, desc, **kw):
    d = dict(BASE); d.update(kw)
    if d['mode'] == 1: d.update(dry=0.0, wet=kw.get('wet', 1.0)) 
    d['in'] = d.pop('in_')
    P.append(dict(name=name, category=cat, description=desc, schema=1, params=d))
S64, S32, S16 = 0, 1, 2
# ---- Romeo Santos (inspired by): intimate, silky, deep, centered-wide
C = "Romeo Santos"
add(C, "Romeo - Lead Vocal", "Espacio elegante y romántico, con una cola suave y envolvente. Ideal para voces principales.", algo=5, decay=2.6, predelay=38, div=S32, triplet=True)
add(C, "Romeo - Intimate", "Cámara corta y cercana: la voz se queda al frente con un halo cálido.", algo=2, room=.35, decay=1.6, predelay=28, div=S64, wet=.24, width=.7, damping=.55)
add(C, "Romeo - Wide Chorus", "Plate ancho y denso para estribillos y coros abiertos.", algo=0, room=.6, decay=3.4, predelay=45, div=S32, width=1.0, spread=1.0, wet=.32)
add(C, "Romeo - Long Tail", "Hall cinematográfico con cola larga que no tapa la dicción.", algo=3, room=.6, decay=5.2, predelay=55, div=S32, lowcut=180, wet=.3, duckon=True, duckdepth=7)
# ---- Raulin Rodriguez: warm, natural, defined attack
C = "Raulin Rodriguez"
add(C, "Raulin - Classic", "Plate cálido y natural de bachata romántica tradicional.", algo=0, decay=2.0, predelay=24, div=S64, warmth=.7, damping=.55, width=.7, early=.6)
add(C, "Raulin - Romantic Lead", "Cola cálida y expresiva, ataque vocal definido.", algo=5, decay=2.5, predelay=30, div=S64, warmth=.68, width=.75)
add(C, "Raulin - Soft Plate", "Plate suave, densidad agradable, agudos redondeados.", algo=0, decay=2.8, predelay=36, div=S32, triplet=True, highcut=7500, damping=.58, wet=.3)
add(C, "Raulin - Emotional Tail", "Cola más larga para finales de frase sentimentales.", algo=2, room=.55, decay=3.8, predelay=42, div=S32, triplet=True, warmth=.7, wet=.3)
# ---- Joe Veras: sweet, soft highs, clean mids
C = "Joe Veras"
add(C, "Joe Veras - Sweet", "Reverb dulce y suave, sin aspereza en agudos.", algo=5, decay=2.2, predelay=34, div=S32, triplet=True, highcut=7200, damping=.6, tone=.5)
add(C, "Joe Veras - Warm Room", "Sala cálida y cercana, muy íntima.", algo=1, room=.4, decay=1.4, predelay=20, div=S64, warmth=.7, wet=.26, width=.7)
add(C, "Joe Veras - Romantic Plate", "Plate romántico con buena separación entre voz seca y ambiente.", algo=0, decay=2.6, predelay=40, div=S32, triplet=True, highcut=7800, damping=.55)
add(C, "Joe Veras - Vocal Depth", "Profundidad sin alejar la voz.", algo=2, room=.5, decay=3.0, predelay=48, div=S32, highcut=7500, damping=.6, wet=.3)
# ---- Bachata 90s: compact, warm, nostalgic, early-reflection heavy
C = "Bachata 90s"
add(C, "90s - Dry Romantic", "Espacio compacto y casi seco, romántico.", algo=1, room=.3, decay=1.1, predelay=15, sync=False, wet=.22, width=.6, highcut=7000, early=.8)
add(C, "90s - Vintage Room", "Sala digital vieja con carácter nostálgico.", algo=4, room=.35, decay=1.6, predelay=18, sync=False, wet=.25, highcut=6500, width=.65, mod=.4)
add(C, "90s - Warm Plate", "Plate moderado, cálido, brillo controlado.", algo=0, room=.4, decay=1.9, predelay=22, sync=False, wet=.26, highcut=7000, warmth=.72)
add(C, "90s - Classic Vocal", "El reverb vocal clásico de los 90, medio y cálido.", algo=4, room=.4, decay=2.1, predelay=26, div=S64, wet=.27, highcut=6800, width=.7, mod=.35)
# ---- Bachata 2000s: denser, defined predelay, wide but centered
C = "Bachata 2000s"
add(C, "2000s - Romantic Lead", "Más densidad que los 90, predelay definido, medios cálidos.", algo=5, room=.5, decay=2.7, predelay=40, div=S32, triplet=True, density=.8, width=.9, wet=.3)
add(C, "2000s - Sweet Plate", "Plate dulce, amplio y con centro firme.", algo=0, room=.5, decay=2.9, predelay=45, div=S32, density=.8, width=.95, wet=.3)
add(C, "2000s - Vocal Hall", "Hall corto con buena profundidad en la voz principal.", algo=3, room=.45, decay=3.4, predelay=50, div=S32, density=.8, width=.95, wet=.28)
add(C, "2000s - Bachata Classic", "Cámara equilibrada, el sonido típico de balada de los 2000.", algo=2, room=.5, decay=2.4, predelay=36, div=S32, triplet=True, density=.78, wet=.3)
# ---- Modern long reverb: sync, wide, long tails with ducking
C = "Modern Bachata"
add(C, "Modern - Long Tail", "Cola larga, profunda y envolvente con ducking suave.", algo=5, room=.65, decay=6.5, predelay=77, div=S16, triplet=True, width=1.0, spread=1.0, density=.85, lowcut=220, damping=.55, duckon=True, duckdepth=9, duckrel=400, wet=.32)
add(C, "Modern - Wide Hall", "Hall amplio y denso para producciones actuales.", algo=3, room=.7, decay=7.0, predelay=115, div=S16, width=1.0, spread=1.0, density=.85, lowcut=220, duckon=True, duckdepth=8, duckrel=450, wet=.3)
add(C, "Modern - Dreamy Vocal", "Cola soñadora con movimiento sutil.", algo=4, room=.65, decay=8.0, predelay=77, div=S16, triplet=True, mod=.5, width=1.0, lowcut=240, highcut=7000, duckon=True, duckdepth=10, duckrel=450, wet=.3)
add(C, "Modern - Emotional Ending", "Para finales de frase: la cola crece en las pausas y se aparta cuando entra la voz.", algo=5, room=.7, decay=9.0, predelay=115, div=S16, width=1.0, spread=1.0, lowcut=250, duckon=True, duckthr=-32, duckdepth=14, duckrel=600, wet=.34)
# ---- Aux / Send & extras
C = "Aux / Send"
add(C, "Bachata - Aux Send", "Punto de partida para canal auxiliar: 100 % wet, 0 % dry.", mode=1, algo=5, decay=2.8, predelay=38, div=S32, triplet=True, wet=1.0)
add(C, "Bachata - Lead Insert", "Para insertar directo en la voz principal.", mode=0, algo=0, decay=2.0, predelay=28, div=S64, wet=.22)
add(C, "Bachata - Background Vocals", "Coros atrás y anchos, con pocas reflexiones tempranas.", mode=1, algo=3, room=.6, decay=3.2, predelay=60, sync=False, width=1.0, early=.3, spread=1.0, wet=1.0)
add(C, "Bachata - Double Tracking", "Ensancha dobles de voz sin agrandar el cuarto.", mode=0, algo=1, room=.35, decay=1.2, predelay=18, sync=False, width=1.0, spread=1.0, mod=.4, wet=.28)
add(C, "Bachata - Falsetto", "Agudos más suaves para falsetes.", mode=1, algo=0, decay=3.6, predelay=42, div=S32, highcut=6800, damping=.62, tone=.45, wet=1.0)
add(C, "Bachata - Vibrato", "Poca modulación para que el vibrato no choque con la cola.", mode=1, algo=5, decay=2.4, predelay=34, div=S32, triplet=True, mod=.1, wet=1.0)
add(C, "Bachata - Ad-libs", "Ad-libs con eco de ambiente que se aparta con ducking.", mode=1, algo=3, room=.6, decay=4.0, predelay=70, sync=False, duckon=True, duckdepth=12, wet=1.0)
add(C, "Bachata - Whispery Vocal", "Cercano y aireado para voces susurradas.", mode=0, algo=2, room=.4, decay=2.2, predelay=24, div=S64, early=.7, highcut=10000, wet=.34)
add(C, "Bachata - Final Phrase", "Cola larga que florece al final de la frase.", mode=1, algo=3, room=.7, decay=6.0, predelay=80, sync=False, duckon=True, duckdepth=14, duckrel=550, wet=1.0)
add(C, "Bachata - Infinite Atmosphere", "Cola muy larga y controlada (decay máx. 14 s, limitada internamente).", mode=1, algo=3, room=.8, decay=14.0, predelay=90, sync=False, lowcut=250, highcut=6500, damping=.6, lowdecay=.2, mod=.6, duckon=True, duckdepth=10, duckrel=600, wet=1.0)
os.makedirs("Source/Presets", exist_ok=True)
json.dump(P, open("Source/Presets/factory_presets.json", "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print(len(P), "presets")
