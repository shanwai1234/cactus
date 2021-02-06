/*
 * Copyright (C) 2009-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include "cactusGlobalsPrivate.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic end functions.
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

EndContents *end_getContents(End *end) {
    return (EndContents *)(end->order ? end+2 : end+1);
}

//static int end_constructP(const void *o1, const void *o2) {
//    return cactusMisc_nameCompare(cap_getName((Cap *) o1), cap_getName((Cap *) o2));
//}

End *end_construct(bool isAttached, Flower *flower) {
    return end_construct3(cactusDisk_getUniqueID(flower_getCactusDisk(flower)), 1,
            isAttached, 1, flower);
}

End *end_construct2(bool side, bool isAttached, Flower *flower) {
    return end_construct3(cactusDisk_getUniqueID(flower_getCactusDisk(flower)), 1,
            isAttached, side, flower);
}

End *end_construct3(Name name, int64_t isStub, int64_t isAttached,
        int64_t side, Flower *flower) {
    End *end;
    end = st_malloc(2*sizeof(End) + sizeof(EndContents));
    end->order = 1;
    (end+1)->order = 0;
    //end->rEnd = st_malloc(sizeof(End));
    //end->rEnd->rEnd = end;
    //end_getContents(end) = st_malloc(sizeof(EndContents));
    //end->rend_getContents(end) = end_getContents(end);

    //end_getContents(end)->orientation = 1;
    end_getContents(end)->side = side;

    end_getContents(end)->nEnd = NULL;

    //end->orientation = 1;
    //end->rEnd->orientation = 0;

    //end->side = side;
    //end->rEnd->side = !side;

    if (!isStub) {
        assert(!isAttached);
    }

    end_getContents(end)->isStub = isStub;
    end_getContents(end)->isAttached = isAttached;

    //end_getContents(end)->rootInstance = NULL;
    end_getContents(end)->name = name;
    //end_getContents(end)->caps = stSortedSet_construct3(end_constructP, NULL);
    end_getContents(end)->firstCap = NULL;
    end_getContents(end)->attachedBlock = NULL;
    end_getContents(end)->group = NULL;
    end_getContents(end)->flower = flower;
    flower_addEnd(flower, end);
    return end;
}

End *end_copyConstruct(End *end, Flower *newFlower) {
    End *end2;
    End_InstanceIterator *iterator;
    Cap *cap;
    Cap *cap2;

    end = end_getPositiveOrientation(end);
    assert(flower_getEnd(newFlower, end_getName(end)) == NULL);

    end2 = end_construct3(end_getName(end), 1, end_isBlockEnd(end) ? 1
            : end_isAttached(end), end_getSide(end), newFlower);
    //Copy the instances.
    iterator = end_getInstanceIterator(end);
    while ((cap = end_getNext(iterator)) != NULL) {
        cap_copyConstruct(end2, cap);
    }
    end_destructInstanceIterator(iterator);

    //Copy any parent child links.
    iterator = end_getInstanceIterator(end);
    while ((cap = end_getNext(iterator)) != NULL) {
        if ((cap2 = cap_getParent(cap)) != NULL) {
            cap_makeParentAndChild(end_getInstance(end2, cap_getName(cap2)),
                    end_getInstance(end2, cap_getName(cap)));
        }
    }
    end_destructInstanceIterator(iterator);

    //Copy root.
    if (end_getRootInstance(end) != NULL) {
        end_setRootInstance(end2, end_getInstance(end2, cap_getName(
                end_getRootInstance(end))));
    }
    return end2;
}

void end_destruct(End *end) {
    Cap *cap;
    //remove from flower.
    flower_removeEnd(end_getFlower(end), end);

    //remove from group.
    end_setGroup(end, NULL);

    //remove instances
    while ((cap = end_getFirst(end)) != NULL) {
        cap_destruct(cap);
    }
    //now the actual instances.
    //stSortedSet_destruct(end_getContents(end)->caps);

    //free(end_getContents(end));
    //free(end->rEnd);
    free(end->order ? end : end_getReverse(end));
}

void end_setBlock(End *end, Block *block) {
    assert(end_getOrientation(end));
    assert(block_getOrientation(block));
    end_getContents(end)->attachedBlock = block;
}

Name end_getName(End *end) {
    return end_getContents(end)->name;
}

bool end_getOrientation(End *end) {
    return end->order; // ^ end_getContents(end)->orienta;
}

End *end_getPositiveOrientation(End *end) {
    return end_getOrientation(end) ? end : end_getReverse(end);
}

End *end_getReverse(End *end) {
    return end->order ? end+1 : end-1; //end->rEnd;
}

bool end_getSide(End *end) {
    return !(end->order ^ end_getContents(end)->side);
}

Flower *end_getFlower(End *end) {
    return end_getContents(end)->flower;
}

Block *end_getBlock(End *end) {
    Block *a = end_getContents(end)->attachedBlock;
    return a == NULL || end_getOrientation(end) ? a : block_getReverse(a);
}

End *end_getOtherBlockEnd(End *end) {
    if (!end_isBlockEnd(end)) {
        return NULL; //the end must be block end to return the other end of a block!
    }
    Block *block = end_getBlock(end);
    assert(block != NULL);
    End *otherEnd = end_getSide(end) ? block_get3End(block) : block_get5End(
            block);
    assert(end_getOrientation(end) == end_getOrientation(otherEnd));
    assert(end != otherEnd);
    return otherEnd;
}

Group *end_getGroup(End *end) {
    return end_getContents(end)->group;
}

int64_t end_getInstanceNumber(End *end) {
    Cap *cap = end_getContents(end)->firstCap;
    int64_t totalCaps = 0;
    while(cap != NULL) {
        totalCaps++; cap = cap_getContents(cap)->nCap;
    }
    return totalCaps;
    //return stSortedSet_size(end_getContents(end)->caps);
}

Cap *end_getInstanceP(End *end, Cap *connectedCap) {
    return connectedCap == NULL ? connectedCap
            : (end_getOrientation(end) ? connectedCap : cap_getReverse(
                    connectedCap));
}

Cap *end_getInstance(End *end, Name name) {
    Cap *cap = end_getContents(end)->firstCap;
    while(cap != NULL) {
        if(cap_getName(cap) == name) {
            return end_getInstanceP(end, cap);
        }
        cap = cap_getContents(cap)->nCap;
    }
    return NULL;

    //CapContents capContents[2];
    //Cap *cap = (Cap *)&capContents; // Very ugly hack
    //cap_getContents(cap)->instance = name;
    //assert(cap_getName(cap) == name);
    //Cap cap;
    //CapContents capContents;
    //cap.capContents = &capContents;
    //capContents.instance = name;
    //return end_getInstanceP(end,
    //        stSortedSet_search(end_getContents(end)->caps, cap));
}

Cap *end_getFirst(End *end) {
    return end_getInstanceP(end, end_getContents(end)->firstCap);
    //return end_getInstanceP(end, stSortedSet_getFirst(end_getContents(end)->caps));
}

Cap *end_getRootInstance(End *end) {
    return NULL;
    //return end_getInstanceP(end, end_getContents(end)->rootInstance);
}

void end_setRootInstance(End *end, Cap *cap) {
    assert(0);
    //end_getContents(end)->rootInstance = cap_getOrientation(cap) ? cap
    //        : cap_getReverse(cap);
}

End_InstanceIterator *end_getInstanceIterator(End *end) {
    End_InstanceIterator *iterator;
    iterator = st_malloc(sizeof(struct _end_instanceIterator));
    iterator->end = end;
    iterator->cap = end_getContents(end)->firstCap;
    //iterator->iterator = stSortedSet_getIterator(end_getContents(end)->caps);
    return iterator;
}

Cap *end_getNext(End_InstanceIterator *iterator) {
    Cap *cap = iterator->cap;
    if(cap == NULL) {
        return NULL;
    }
    iterator->cap = cap_getContents(cap)->nCap;
    return end_getInstanceP(iterator->end, cap);
    //return end_getInstanceP(iterator->end, stSortedSet_getNext(
    //        iterator->iterator));
}

Cap *end_getPrevious(End_InstanceIterator *iterator) {
    assert(0);
    return NULL;
    //return end_getInstanceP(iterator->end, stSortedSet_getPrevious(
    //        iterator->iterator));
}

End_InstanceIterator *end_copyInstanceIterator(End_InstanceIterator *iterator) {
    assert(0);
    End_InstanceIterator *iterator2;
    iterator2 = st_malloc(sizeof(struct _end_instanceIterator));
    iterator2->end = iterator->end;
    iterator2->cap = iterator->cap;
    //iterator2->iterator = stSortedSet_copyIterator(iterator->iterator);
    return iterator2;
}

void end_destructInstanceIterator(End_InstanceIterator *iterator) {
    //stSortedSet_destructIterator(iterator->iterator);
    free(iterator);
}

bool end_isBlockEnd(End *end) {
    return !end_isStubEnd(end);
}

bool end_isStubEnd(End *end) {
    return end_getContents(end)->isStub;
}

bool end_isAttached(End *end) {
    return end_getContents(end)->isAttached;
}

bool end_isFree(End *end) {
    return !end_isAttached(end);
}

void end_setGroup(End *end, Group *group) {
    if (end_getGroup(end) != NULL) {
        group_removeEnd(end_getGroup(end), end);
    }
    end_getContents(end)->group = group;
    if (group != NULL) {
        group_addEnd(group, end);
    }
}

void end_makeAttached(End *end) {
    assert(end_isStubEnd(end));
    assert(end_isFree(end));
    assert(flower_getName(end_getFlower(end)) == 0);
    if(end_getGroup(end) != NULL) {
        assert(group_isLeaf(end_getGroup(end)));
    }
    end_getContents(end)->isAttached = 1;
}

void end_check(End *end) {
    //Check is connected to flower properly
    assert(flower_getEnd(end_getFlower(end), end_getName(end)) == end_getPositiveOrientation(end));

    //check end is part of group..
    Group *group = end_getGroup(end);
    assert(group != NULL);
    assert(group_getEnd(group, end_getName(end)) == end_getPositiveOrientation(end));

    if (end_isBlockEnd(end)) {
        assert(!end_isStubEnd(end));
        assert(end_isFree(end));
        //Check block..
        Block *block = end_getBlock(end);
        assert(block != NULL);
        assert(block_getOrientation(block) == end_getOrientation(end));
        //check not attached
        assert(end_isFree(end));
        assert(!end_isAttached(end));
        //Check sides correspond..
        if (end_getSide(end)) {
            assert(block_get5End(block) == end);
        } else {
            assert(block_get3End(block) == end);
        }
    } else {
        assert(end_isStubEnd(end)); //Is stub end:
        //there must be no attached block.
        assert(end_getBlock(end) == NULL);
        Group *parentGroup = flower_getParentGroup(end_getFlower(end));
        if (parentGroup != NULL) {
            // if attached the is inherited from a parent flower to the containing flower.
            End *parentEnd = group_getEnd(parentGroup, end_getName(end));
            assert(end_getOrientation(parentEnd));
            if (end_isAttached(end)) {
                assert(parentEnd != NULL);
            }
            if (parentEnd != NULL) {
                assert(end_getSide(parentEnd) == end_getSide(end_getPositiveOrientation(end)));
            }
        }
    }

    //Check reverse, not comprehensively, perhaps.
    End *rEnd = end_getReverse(end);
    assert(rEnd != NULL);
    assert(end_getReverse(rEnd) == end);
    assert(end_getOrientation(end) == !end_getOrientation(rEnd));
    assert(end_getSide(end) == !end_getSide(rEnd));
    assert(end_getName(end) == end_getName(rEnd));
    assert(end_getInstanceNumber(end) == end_getInstanceNumber(rEnd));
    assert(end_isAttached(end) == end_isAttached(rEnd));
    assert(end_isStubEnd(end) == end_isStubEnd(rEnd));
    if (end_getRootInstance(end) == NULL) {
        assert(end_getRootInstance(rEnd) == NULL);
    } else {
        assert(end_getRootInstance(end) == cap_getReverse(end_getRootInstance(rEnd)));
    }
    if (end_getInstanceNumber(end) > 0) {
        assert(end_getFirst(end) == cap_getReverse(end_getFirst(rEnd)));
    }

    //Check has tree if built_trees set
    if (flower_builtTrees(end_getFlower(end)) && end_getInstanceNumber(end) > 0) {
        assert(end_getRootInstance(end) != NULL);
    }

    //For each segment calls segment_check.
    End_InstanceIterator *iterator = end_getInstanceIterator(end);
    Cap *cap;
    while ((cap = end_getNext(iterator)) != NULL) {
        cap_check(cap);
    }
    end_destructInstanceIterator(iterator);
}

Cap *end_getCapForEvent(End *end, Name eventName) {
    /*
     * Get the cap for a given event.
     */
    End_InstanceIterator *it = end_getInstanceIterator(end);
    Cap *cap;
    while ((cap = end_getNext(it)) != NULL) {
        if (event_getName(cap_getEvent(cap)) == eventName) {
            end_destructInstanceIterator(it);
            return cap;
        }
    }
    end_destructInstanceIterator(it);
    return NULL;
}

/*
 * Private functions
 */

void end_addInstance(End *end, Cap *cap) {
    assert(cap_getContents(cap)->nCap == NULL);
    cap_getContents(cap)->nCap = end_getContents(end)->firstCap;
    end_getContents(end)->firstCap = cap_getPositiveOrientation(cap);
    //stSortedSet_insert(end_getContents(end)->caps, cap_getPositiveOrientation(cap));
}

void end_removeInstance(End *end, Cap *cap) {
    Cap **capP = &(end_getContents(end)->firstCap);
    while(*capP != NULL) {
        if(cap_getName(cap) == cap_getName(*capP)) {
            (*capP) = cap_getContents(*capP)->nCap; // Splice it out
            return;
        }
        capP = &(cap_getContents(*capP)->nCap);
    }
    //stSortedSet_remove(end_getContents(end)->caps, cap);
}

void end_setFlower(End *end, Flower *flower) {
    flower_removeEnd(end_getFlower(end), end);
    end_getContents(end)->flower = flower;
    flower_addEnd(flower, end);
}

/*
 * Serialisation functions.
 */

void end_writeBinaryRepresentationP(Cap *cap, void(*writeFn)(const void * ptr,
        size_t size, size_t count)) {
    int64_t i;
    cap_writeBinaryRepresentation(cap, writeFn);
    for (i = 0; i < cap_getChildNumber(cap); i++) {
        end_writeBinaryRepresentationP(cap_getChild(cap, i), writeFn);
    }
}

void end_writeBinaryRepresentation(End *end, void(*writeFn)(const void * ptr,
        size_t size, size_t count)) {
    End_InstanceIterator *iterator;
    Cap *cap;

    assert(end_getOrientation(end));
    cap = end_getRootInstance(end);
    int64_t endType = cap == NULL ? CODE_END_WITHOUT_PHYLOGENY : CODE_END_WITH_PHYLOGENY;
    binaryRepresentation_writeElementType(endType, writeFn);
    binaryRepresentation_writeName(end_getName(end), writeFn);
    binaryRepresentation_writeBool(end_isStubEnd(end), writeFn);
    binaryRepresentation_writeBool(end_isAttached(end), writeFn);
    binaryRepresentation_writeBool(end_getSide(end), writeFn);

    if (cap == NULL) {
        iterator = end_getInstanceIterator(end);
        while ((cap = end_getNext(iterator)) != NULL) {
            assert(cap_getParent(cap) == NULL);
            cap_writeBinaryRepresentation(cap, writeFn);
        }
        end_destructInstanceIterator(iterator);
    } else {
        end_writeBinaryRepresentationP(cap, writeFn);
    }
    binaryRepresentation_writeElementType(endType, writeFn);
}

End *end_loadFromBinaryRepresentation(void **binaryString, Flower *flower) {
    End *end;
    Name name;
    int64_t isStub;
    int64_t isAttached;
    int64_t side;

    end = NULL;
    if (binaryRepresentation_peekNextElementType(*binaryString)
            == CODE_END_WITHOUT_PHYLOGENY) {
        binaryRepresentation_popNextElementType(binaryString);
        name = binaryRepresentation_getName(binaryString);
        isStub = binaryRepresentation_getBool(binaryString);
        isAttached = binaryRepresentation_getBool(binaryString);
        side = binaryRepresentation_getBool(binaryString);
        end = end_construct3(name, isStub, isAttached, side, flower);
        while (cap_loadFromBinaryRepresentation(binaryString, end) != NULL)
            ;
        assert(binaryRepresentation_peekNextElementType(*binaryString)  == CODE_END_WITHOUT_PHYLOGENY);
        binaryRepresentation_popNextElementType(binaryString);
    } else {
        if (binaryRepresentation_peekNextElementType(*binaryString)
                == CODE_END_WITH_PHYLOGENY) {
            binaryRepresentation_popNextElementType(binaryString);
            name = binaryRepresentation_getName(binaryString);
            isStub = binaryRepresentation_getBool(binaryString);
            isAttached = binaryRepresentation_getBool(binaryString);
            side = binaryRepresentation_getBool(binaryString);
            end = end_construct3(name, isStub, isAttached, side, flower);
            end_setRootInstance(end, cap_loadFromBinaryRepresentation(
                    binaryString, end));
            while (cap_loadFromBinaryRepresentation(binaryString, end) != NULL)
                ;
            assert(binaryRepresentation_peekNextElementType(*binaryString)  == CODE_END_WITH_PHYLOGENY);
            binaryRepresentation_popNextElementType(binaryString);
        }
    }

    return end;
}

uint64_t end_hashKey(const void *o) {
    return end_getName((End *) o);
}

int end_hashEqualsKey(const void *o, const void *o2) {
    End *end1 = (End *) o;
    End *end2 = (End *) o2;
    return end_getName(end1) == end_getName(end2) && end_getOrientation(end1)
            == end_getOrientation(end2);
}
