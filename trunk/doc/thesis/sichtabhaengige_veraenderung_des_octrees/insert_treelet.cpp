 ...
 Treelet* treelet                         = _treelets[tve._treeletGid];
 gloostId parentTreeletGid                = treelet->getParentTreeletGid();
 gloostId parentTreeletLeafPosition       = treelet->getParentTreeletLeafPosition();
 gloostId parentTreeletLeafParentPosition = treelet->getParentTreeletLeafsParentPosition();
 gloostId parentTreeletLeafIdx            = treelet->getParentTreeletLeafIdx();

 Treelet* parentTreelet            = _treelets[parentTreeletGid];
 unsigned incoreLeafPosition       = parentTreelet->getSlotGid()
                                   * _numNodesPerTreelet+parentTreeletLeafPosition;
 unsigned incoreLeafParentPosition = parentTreelet->getSlotGid()
                                   * _numNodesPerTreelet + parentTreeletLeafParentPosition;

 // copy root node of new Treelet to the leaf of parents Treelet
 _incoreBuffer[incoreLeafPosition] = _incoreBuffer[incoreNodePosition];

 // update leaf mask of leafs parent so that the leaf is no leaf anymore
 _incoreBuffer[incoreLeafParentPosition].setLeafMaskFlag(parentTreeletLeafIdx, false);

 // update first child position within leaf/root (relative value within the incore buffer)
 _incoreBuffer[incoreLeafPosition].setFirstChildIndex( (int)(incoreNodePosition+1)
                                                      -(int)incoreLeafPosition);

 // mark incore slot of parent to be uploaded to device memory
 markIncoreSlotForUpload(parentTreelet->getSlotGid());

 // note treeletGid to parent position within the _childTreeletsInIncoreBuffer
 _childTreeletsInIncoreBuffer[parentTreeletGid].insert(tve._treeletGid);

 return true;
}
