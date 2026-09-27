"""STEP assembly -> GLB with the STEP's part colours (OpenCascade XCAF). Usage: step2glb.py IN.stp OUT.glb"""

import sys

from OCP.BRepMesh import BRepMesh_IncrementalMesh
from OCP.IFSelect import IFSelect_RetDone
from OCP.Message import Message_ProgressRange
from OCP.RWGltf import RWGltf_CafWriter
from OCP.STEPCAFControl import STEPCAFControl_Reader
from OCP.TCollection import TCollection_AsciiString, TCollection_ExtendedString
from OCP.TColStd import TColStd_IndexedDataMapOfStringString
from OCP.TDF import TDF_LabelSequence
from OCP.TDocStd import TDocStd_Document
from OCP.XCAFDoc import XCAFDoc_DocumentTool

src, dst = sys.argv[1:3]
doc = TDocStd_Document(TCollection_ExtendedString("doc"))
r = STEPCAFControl_Reader()
r.SetColorMode(True)
r.SetNameMode(True)
assert r.ReadFile(src) == IFSelect_RetDone, "read failed"
r.Transfer(doc)
tool = XCAFDoc_DocumentTool.ShapeTool_s(doc.Main())
labels = TDF_LabelSequence()
tool.GetFreeShapes(labels)
for i in range(1, labels.Length() + 1):
    BRepMesh_IncrementalMesh(tool.GetShape_s(labels.Value(i)), 0.02, False, 0.3, True)  # 0.02 mm chord
w = RWGltf_CafWriter(TCollection_AsciiString(dst), True)  # binary .glb
w.Perform(doc, TColStd_IndexedDataMapOfStringString(), Message_ProgressRange())
print("wrote", dst)
