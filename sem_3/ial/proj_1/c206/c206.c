
/* c206.c **********************************************************}
{* Téma: Dvousměrně vázaný lineární seznam
**
**                   Návrh a referenční implementace: Bohuslav Křena, říjen 2001
**                            Přepracované do jazyka C: Martin Tuček, říjen 2004
**                                            Úpravy: Kamil Jeřábek, září 2018
**
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován proměnnou
** typu tDLList (DL znamená Double-Linked a slouží pro odlišení
** jmen konstant, typů a funkcí od jmen u jednosměrně vázaného lineárního
** seznamu). Definici konstant a typů naleznete v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu
** s výše uvedenou datovou částí abstrakce tvoří abstraktní datový typ
** obousměrně vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu před prvním použitím,
**      DLDisposeList ... zrušení všech prvků seznamu,
**      DLInsertFirst ... vložení prvku na začátek seznamu,
**      DLInsertLast .... vložení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zruší první prvek seznamu,
**      DLDeleteLast .... zruší poslední prvek seznamu,
**      DLPostDelete .... ruší prvek za aktivním prvkem,
**      DLPreDelete ..... ruší prvek před aktivním prvkem,
**      DLPostInsert .... vloží nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vloží nový prvek před aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... přepíše obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na další prvek seznamu,
**      DLPred .......... posune aktivitu na předchozí prvek seznamu,
**      DLActive ........ zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce
** explicitně uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam 
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int errflg;
int solved;

void DLError() {
/*
** Vytiskne upozornění na to, že došlo k chybě.
** Tato funkce bude volána z některých dále implementovaných operací.
**/	
    printf ("*ERROR* The program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální proměnná -- příznak ošetření chyby */
    return;
}

void DLInitList (tDLList *L) {
/*
** Provede inicializaci seznamu L před jeho prvním použitím (tzn. žádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádět nad již inicializovaným
** seznamem, a proto tuto možnost neošetřujte. Vždy předpokládejte,
** že neinicializované proměnné mají nedefinovanou hodnotu.
**/
    L->First = NULL;
    L->Act = NULL;
    L->Last = NULL;
}

void DLDisposeList (tDLList *L) {
/*
** Zruší všechny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Rušené prvky seznamu budou korektně
** uvolněny voláním operace free. 
**/
    if (L->First == NULL) {
        L->Last = L->Act = NULL;
        return;
    }
    tDLElemPtr tmp;

    while (L->First != NULL) {
        tmp = L->First;
        L->First = L->First->rptr;
        free(tmp);
    }
    L->First = L->Act = L->Last = NULL;
}

void DLInsertFirst (tDLList *L, int val) {
/*
** Vloží nový prvek na začátek seznamu L.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    tDLElemPtr	tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL) {
        DLError();
        return;
    }

    if (L->Last == NULL) {
        L->Last = tmp;
    } else {
        L->First->lptr = tmp;
    }

    tmp->data = val;
    tmp->lptr = NULL;
    tmp->rptr = L->First;
    L->First = tmp;
}

void DLInsertLast(tDLList *L, int val) {
/*
** Vloží nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    tDLElemPtr	tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL) {
        DLError();
        return;
    }

    if (L->First == NULL) {
        L->First = tmp;
    } else {
        L->Last->rptr = tmp;
    }
    tmp->lptr = L->Last;
    tmp->data = val;
    tmp->rptr = NULL;
    L->Last = tmp;
}

void DLFirst (tDLList *L) {
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
	
    L->Act = L->First;
}

void DLLast (tDLList *L) {
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný příkaz (nepočítáme-li return),
** aniž byste testovali, zda je seznam L prázdný.
**/
    L->Act = L->Last;
}

void DLCopyFirst (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
    if (L->First == NULL) {
        DLError();
        return;
    }
    *val = L->First->data;
}

void DLCopyLast (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
    if (L->Last == NULL) {
        DLError();
        return;
    }
    *val = L->Last->data;
}

void DLDeleteFirst (tDLList *L) {
/*
** Zruší první prvek seznamu L. Pokud byl první prvek aktivní, aktivita 
** se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/
    if (L->First == NULL) {
        return;
    }

    tDLElemPtr tmp = L->First;

    if (L->Act == tmp) { //zrusenie aktivity
        L->Act = NULL;
    }

    if (L->First == L->Last) { //je tam iba 1 prvok
        L->First = L->Last = NULL;
    } else { //viac prvkov
        L->First = tmp->rptr;
        L->First->lptr = NULL;
    }

    free(tmp);
}

void DLDeleteLast (tDLList *L) {
/*
** Zruší poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se neděje.
**/
	if (L->First == NULL) {
        return;
    }

    tDLElemPtr tmp = L->Last;

    if (L->Act == tmp) { //zrusenie aktivity
        L->Act = NULL;
    }

    if (L->First == L->Last) { //je tam iba 1 prvok
        L->First = L->Last = NULL;
    } else { //viac prvkov
        L->Last = tmp->lptr;
        L->Last->rptr = NULL;
    }

    free(tmp);

}

void DLPostDelete (tDLList *L) {
/*
** Zruší prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se neděje.
**/
    if (L->Act == NULL || L->Act == L->Last) {
        return;
    }
    tDLElemPtr tmp = L->Act->rptr;
    if (tmp == L->Last) { //mazem posledny prvok
        L->Last = tmp ->lptr;
        L->Last->rptr = NULL;
    } else { //mazem prvok "v strede"
        L->Act->rptr = tmp->rptr;
        tmp->rptr->lptr = L->Act;
    }
    free(tmp);
}

void DLPreDelete (tDLList *L) {
/*
** Zruší prvek před aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se neděje.
**/
    if (L->Act == NULL || L->Act == L->First) {
        return;
    }
    tDLElemPtr tmp = L->Act->lptr;
    if (tmp == L->First) { //mazem prvy prvok
        L->First = tmp ->rptr;
        L->First->lptr = NULL;
    } else { //mazem prvok "v strede"
        L->Act->lptr = tmp->lptr;
        tmp->lptr->rptr = L->Act;
    }
    free(tmp);
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vloží prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/
    if (L->Act == NULL) {
        return;
    }
    tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL) {
        DLError();
        return;
    }

    tmp->data = val;
    tmp->lptr = L->Act;

    if(L->Act == L->Last) { //vkladanie na koniec
        L->Last = tmp;
        tmp->rptr = NULL;
    } else { //vkladanie "medzi"
        tmp->rptr = L->Act->rptr;
        tmp->rptr->lptr = tmp;
    }
    L->Act->rptr = tmp;

}

void DLPreInsert (tDLList *L, int val) {
/*
** Vloží prvek před aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se neděje.
** V případě, že není dostatek paměti pro nový prvek při operaci malloc,
** volá funkci DLError().
**/


    if (L->Act == NULL) {
        return;
    }
    tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL) {
        DLError();
        return;
    }

    tmp->data = val;
    tmp->rptr = L->Act;

    if(L->Act == L->First) { //vkladanie na zaciatok
        L->First = tmp;
        tmp->lptr = NULL;
    } else { //vkladanie "medzi"
        tmp->lptr = L->Act->lptr;
        tmp->lptr->rptr = tmp;

    }
    L->Act->lptr = tmp;
}

void DLCopy (tDLList *L, int *val) {
/*
** Prostřednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
    if (L->Act == NULL) {
        DLError();
        return;
    }
    *val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** Přepíše obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedělá nic.
**/
    if (L->Act == NULL) {
        return;
    }
    L->Act->data = val;
}

void DLSucc (tDLList *L) {
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
**/
    if (L->Act == NULL) {
        return;
    }

    L->Act = L->Act->rptr;
}


void DLPred (tDLList *L) {
/*
** Posune aktivitu na předchozí prvek seznamu L.
** Není-li seznam aktivní, nedělá nic.
** Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
**/
    if (L->Act == NULL) {
        return;
    }

    L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {
/*
** Je-li seznam L aktivní, vrací nenulovou hodnotu, jinak vrací 0.
** Funkci je vhodné implementovat jedním příkazem return.
**/
    return L->Act != NULL;
}

/* Konec c206.c*/
