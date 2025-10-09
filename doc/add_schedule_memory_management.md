# Uploading a Graph to LM32 Firmware: Required Datasets

To fulfill the simple use case of uploading a graph to the LM32 firmware (FPGA), you need to send several datasets to the FPGA so it can execute the schedule/graph. Based on your code and typical datamaster architecture, here’s what you need to upload:

---

## 1. Node Data

- **What:** Serialized data for each node in the graph (timing messages, commands, blocks, etc.).
- **Where:** Each node is uploaded to a specific memory region, typically determined by its CPU and address (see `atUp.adrConv()` and allocation logic).
- **How:** The data is written in blocks (see `ew.vb.insert(...)` and address generation in `gatherUploadVector()`).

---

## 2. Management Table (Mgmt Nodes)

- **What:** Special nodes for management and control, separate from normal schedule nodes.
- **Where:** Uploaded to a dedicated management memory region (see `atUp.getMgmtTable()` and related address logic).
- **How:** Similar to node data, but using the management table and offsets.

---

## 3. Bitmaps (BMP)

- **What:** Bitmaps indicating allocation status or other metadata for each CPU’s memory pool.
- **Where:** Uploaded to the start of each CPU’s memory region (see `getBmp()` and address logic).
- **How:** Written as a block at the beginning of each memory region.

---

## 4. Global Meta Information

- **What:** Meta information about the management linked list, such as:
  - Start pointer
  - Total size
  - Group table size
  - Coverage table size
  - Reference table size
- **Where:** Uploaded to a fixed offset in the management memory region (see `modAdrBase` and `T_META_*` constants).
- **How:** Written as a block of 5 × 32-bit words.

---

## 5. Management Binary Data

- **What:** Serialized and compressed group, coverage, and reference tables (see `generateMgmtData()`).
- **Where:** Uploaded to the management memory region.
- **How:** Allocated and populated via `atUp.allocateMgmt()` and `atUp.populateMgmt()`.

---

## 6. Modification Info

- **What:** Information about which CPUs/nodes have been modified (for partial updates).
- **Where:** Uploaded as part of the management or node data.
- **How:** Created via `createSchedModInfo()`.

---

## Summary Table

| Dataset                | Purpose                          | Where (Memory Region)         | How to Upload                |
|------------------------|----------------------------------|-------------------------------|------------------------------|
| Node Data              | Schedule execution               | Per-node, per-CPU region      | `gatherUploadVector()`       |
| Management Table       | Control/meta nodes               | Management memory region      | `getMgmtTable()`             |
| Bitmaps (BMP)          | Allocation status                | Start of each CPU region      | `getBmp()`                   |
| Global Meta Info       | Linked list/meta pointers        | Mgmt region, fixed offset     | `writeLeNumberToBeBytes()`   |
| Management Binary Data | Group/Coverage/Reference tables  | Mgmt region                   | `generateMgmtData()`         |
| Modification Info      | Track changes for upload         | Mgmt/node data                | `createSchedModInfo()`       |

---

## Minimal Upload Sequence

1. **Prepare all node and management data** (serialize, compress if needed).
2. **Generate address and data vectors** for all regions (nodes, mgmt, bitmaps, meta).
3. **Send all data blocks to the FPGA** using the appropriate protocol (e.g., Etherbone write cycles).
4. **Trigger execution** (if required by firmware).

---

## In summary

You must upload all node data, management/meta tables, bitmaps, and global meta info to the correct memory regions on the FPGA. The code in `gatherUploadVector()`, `generateMgmtData()`, and related functions shows how these datasets are prepared and where they are written. This ensures the LM32 firmware has everything it needs to execute the uploaded graph.