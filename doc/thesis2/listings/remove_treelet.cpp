 ...  
 gloostId parentTreeletGid                = treelet->getParentTreeletGid();
 gloostId parentTreeletLeafPosition       = treelet->getParentTreeletLeafPosition();
 gloostId parentTreeletLeafParentPosition = treelet->getParentTreeletLeafsParentPosition();
 gloostId parentTreeletLeafIdx            = treelet->getParentTreeletLeafIdx();

 gloost::gloostId slotGid       = treelet->getSlotGid();
 gloost::gloostId parentSlotGid = _treelets[parentTreeletGid]->getSlotGid();

 unsigned incoreLeafPosition       = parentSlotGid
                                   *_numNodesPerTreelet + parentTreeletLeafPosition;
 unsigned incoreLeafParentPosition = parentSlotGid
                                   *_numNodesPerTreelet + parentTreeletLeafParentPosition;

  // kopiere den originalen Blattknoten an seine Position 
 _incoreBuffer[incoreLeafPosition]
                = getTreelet(parentTreeletGid)->getNodeForIndex(parentTreeletLeafPosition);

 // aktualisiere die Leaf-Mask des Elternknotens (innerer Knoten wird wieder Blatknoten)
 _incoreBuffer[incoreLeafParentPosition].setLeafMaskFlag(parentTreeletLeafIdx, true);

 // notiere den Slot des Eltern-Treelets damit er serverseitig Aktualisiert wird
 markIncoreSlotForUpload(parentSlotGid);

 // entferne information des Treelets aus den Slotinformationen
 _slots[slotGid] = SlotInfo();

 // füge den Slot zur Menge der freien Slots dazu
 _freeIncoreSlots.push(slotGid);


 return true;
}
