# Data formats used by Psi Engine
### Psi Mesh (.msh):
| Data           | Size [bytes]                     |
| -------------- | -------------------------------- |
| (V)ertex count | 8                                |
| (I)ndex count  | 8                                |
| Bounding box   | 24                               |
| MD5 Hash of /\ | 16                               |
| -------------- | -------------------------------- |
| Vertices       | V * sizeof(psi_rndr::VertexData) |
| Indices        | I * sizeof(uint32_t)             |
| MD5 Hash of /\ | 16                               |
