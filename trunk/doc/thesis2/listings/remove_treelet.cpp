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

 // copy original node/leaf to incore leaf position
 _incoreBuffer[incoreLeafPosition]
                = getTreelet(parentTreeletGid)->getNodeForIndex(parentTreeletLeafPosition);

 // update leaf mask of leafs parent so that the leaf is a leaf again
 _incoreBuffer[incoreLeafParentPosition].setLeafMaskFlag(parentTreeletLeafIdx, true);

 // mark incore slot of parent to be uploaded to device memory
 markIncoreSlotForUpload(parentSlotGid);

 // clear slot info
 _slots[slotGid] = SlotInfo();

 // add slots to available/free slots
 _freeIncoreSlots.push(slotGid);

 // remove assoziation from treelet gid to slot
 treelet->setSlotGid(-1);

 // remove entry of treelet within parents list of incore child treelets
 _childTreeletsInIncoreBuffer[parentTreeletGid].erase(treeletGid);

 return true;
}
