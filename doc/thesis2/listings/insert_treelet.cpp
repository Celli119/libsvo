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


 // kopiere den Wurzelknoten des neuen Treelets auf das Blatt des Eltern-Treelets
 _incoreBuffer[incoreLeafPosition] = _incoreBuffer[incoreNodePosition];

 // aktualisiere die Leaf-Mask des Elternknotens des Blattes (Blattknoten wird innerer Knoten)
 _incoreBuffer[incoreLeafParentPosition].setLeafMaskFlag(parentTreeletLeafIdx, false);

 // aktualisiere den First-Child-Index des neuen inneren Knotes auf die zweite Position
 // im neuen Treelet (das war das erste Kind des Wurzelknotens)
 _incoreBuffer[incoreLeafPosition].setFirstChildIndex( (int)(incoreNodePosition+1)
                                                      -(int)incoreLeafPosition);

 // notiere den Slot des Eltern-Treelets damit er serverseitig Aktualisiert wird
 markIncoreSlotForUpload(parentTreelet->getSlotGid());

 // notiere den Slot des neuen Treelets damit er serverseitig Aktualisiert wird
 markIncoreSlotForUpload(_treelets[tve._treeletGid]->getSlotGid());

 // notiere dass das Eltern-Treelet ein neues Kind-Treelet im Incore-Buffer besitzt
 _childTreeletsInIncoreBuffer[parentTreeletGid].insert(tve._treeletGid);

 return true;
}
