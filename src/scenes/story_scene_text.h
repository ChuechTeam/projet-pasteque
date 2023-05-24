#ifndef PROJET_PASTEQUE_STORY_SCENE_TEXT_H
#define PROJET_PASTEQUE_STORY_SCENE_TEXT_H

#include "board.h"

// /C = user must enter to continue, text erased
// /P = pause (300ms)
// /N = narrator color
// /S = story color

#define STORY_TEXT_1 \
"/STout commence dans une ville banale, déserte, entourée de nombreux bâtiments rectangulaires, hexagonaux, polygonaux, aux bords tranchants comme des rasoirs. C’est d’ailleurs le sujet d’une publicité éblouissante affichée sur un écran accroché à un des bâtiments ; la seule source de lumière dans cette sombre nuit de nouvelle lune./P\n\n"\
"Cet étrange paysage sera bientôt le quotidien d'Inori./P Auparavant, il travaillait dans une petite entreprise de télé-restaurant, mais la société a dû renvoyer 5% de ses 20 employés pour des raisons financières./P Une stratégie bien rusée pour ne pas payer les frais de démission./C\n"\
"Endetté de 1505€, la situation est plus qu’urgente pour Inori./P Son périple commence devant l’entrée d’un bâtiment d’un gris uniforme, rectangulaire mais parsemé de creux autour des bords, à l’image d’une brique de lait vide regonflée d’air./P Inori franchit la porte d’entrée, et là, c’est le choc…/C\n"\
"/N…Et là QUOI ?/P Quel choc ?/P Y’a pas la suite ?!/P Attends, elle doit bien être quelque part…/P Non, j’ai bien regardé le sommaire et il n’y a qu’une page. C'est pas possible…/P L'histoire n'a pas pu se volatiliser comme ça…/P Bon, tu sais quoi, j’ai trouvé ce jeu dans un vide-greniers, ça devrait t’occuper pendant que je cherche la suite./P Perso, ce jeu m’a fait péter une vitre, tellement que j’ai dû payer 655€ de réparations./P Depuis, j’y touche plus, mais toi, tu peux y arriver."

#define STORY_TEXT_2 \
"/NAh…/P Voilà, j’ai retrouvé la suite./P Tu sais où elle était cachée ?/P Dans un autre livre !/P Et fallait l’acheter en prime !/P Heureusement qu’on a des technologies de livraison par trans-matérialisation, sans quoi, t’aurais attendu jusqu’à demain !/P T’en fais pas, celui-ci, il est bien plus gros, alors tiens-toi bien prêt à appendre la suite de l’histoire./C\n"\
"/SInori est pris d’une stupeur effroyable…/P L’intérieur du bâtiment était à moitié aspiré par une sorte de trou noir, faisant léviter le sol en mille morceaux./P Il distinguait une douce voix à sa gauche :/P\n"\
"« …Dernier modèle de modulateur spatial, un véritable plaisir oculaire pour petits et grands ! »/P\n"\
"Ce qui semblait être une famille se tenait devant le modulateur, accompagnée d’une jeune femme présentant ce produit en toute tranquillité./P Inori reprit ses esprits, et engagea la parole à l’accueil pour accomplir sa destinée :/C\n"\
"« Bonjour ! Dites, vous auriez… Des postes à remplir ?/P\n"\
"— Euh… ça te dérangerait d’être un peu plus clair ?/P\n"\
"— Bah, avoir un job chez vous./P\n"\
"— Ça tombe bien, on avait besoin d’un testeur pour nos produits ! Attends une seconde… Je cherche dans mes dossiers…/C\n"\
"/N…Non mais quoi encore ?/P Je te jure, le livre il fait 100 pages, et j’ai pas accès aux 98 autres pages, c’est bloqué, comme de la glu !/P Et c’est dans ma main, hein…/P Attends, j’arrive un lire un truc : « Veuillez patienter un certain temps avant de continuer. »/P\
 Ah mais oui, c’est vrai que c’est « immersif » ce machin, mais c’est pas foutu de donner les secondes exactes !/P Je te laisse faire joujou avec ton nouveau jouet le temps que ça se déverrouille, moi, je vais me faire un p’tit chocolat chaud avec éclats de cookie."\

#define STORY_TEXT_3 \
"/NTe revoilà enfin./P Tu te souviens de mon chocolat chaud ?/P Pour une raison que j’ignore, le chocolat a juste DISPARU !/P\
 Heureusement que tu m’as pas vu manger des pépites de cookies comme un demeuré./P\
 Enfin bon, c’est pas comme si tu pouvais me voir de base./P Prêt pour l’histoire ?/P Ça c’est débloqué !/C\n"\
"/SAu bout de quelques longues minutes de pure bureaucratie, l’hôte d’accueil revint vers Inori :/P\n"\
"« Eh bien, je crains que ce ne sera pas possible pour toi. On va pas se mentir, tes antécédents sont assez gênants, pour ainsi dire./P\n"\
"— Quoi ? s’exclama Inori./P\n"\
"— QUOICOUBEH !! cria l’hôte/C\n"\
"/N./P./P./PNon. Je/P./P./P. J-Je/P./P./P. Désolé, mais…/P Tu…/P Tu dois me laisser une seconde./P Le livre, il…/P Il a fait un bruit tellement strident./P"\
" Les vitres se sont encore brisées…/P Et…/P Il a dit le Mot./P Laisse-moi tranquille, juste un instant…"

#define STORY_TEXT_4 "C'est fini lol"

static const char* storyTexts[] = {STORY_TEXT_1, STORY_TEXT_2, STORY_TEXT_3, STORY_TEXT_4};
static const BoardSizePreset storyPresets[] = {BSP_MEDIUM, BSP_MEDIUM, BSP_SMALL};

#endif //PROJET_PASTEQUE_STORY_SCENE_TEXT_H
