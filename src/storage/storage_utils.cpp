#include "storage_utils.h"

#include <string>
#include <vector>

std::vector<char> serialize(TableMetadata &metadata) { //
  std::vector<char> result;

  // Serialize table_name (length-prefixed string)
  uint32_t name_length = metadata.table_name.length();
  result.insert(result.end(), reinterpret_cast<char *>(&name_length),
                reinterpret_cast<char *>(&name_length) + sizeof(uint32_t));
  result.insert(result.end(), metadata.table_name.begin(),
                metadata.table_name.end());

  // Serialize first_page_id
  result.insert(result.end(), reinterpret_cast<char *>(&metadata.first_page_id),
                reinterpret_cast<char *>(&metadata.first_page_id) +
                    sizeof(page_id_t));

  // Serialize schema - number of columns
  const auto &columns = metadata.schema.get_columns();
  uint32_t num_columns = columns.size();
  result.insert(result.end(), reinterpret_cast<char *>(&num_columns),
                reinterpret_cast<char *>(&num_columns) + sizeof(uint32_t));

  // Serialize each column
  for (const auto &col : columns) {
    // Column name (length-prefixed)
    uint32_t col_name_length = col.name.length();
    result.insert(result.end(), reinterpret_cast<char *>(&col_name_length),
                  reinterpret_cast<char *>(&col_name_length) +
                      sizeof(uint32_t));
    result.insert(result.end(), col.name.begin(), col.name.end());
    // Column type (as uint32_t)
    uint32_t type_val = static_cast<uint32_t>(col.type);
    result.insert(result.end(), reinterpret_cast<char *>(&type_val),
                  reinterpret_cast<char *>(&type_val) + sizeof(uint32_t));
    // Column length
    uint32_t col_length = col.length;
    result.insert(result.end(), reinterpret_cast<char *>(&col_length),
                  reinterpret_cast<char *>(&col_length) + sizeof(uint32_t));
  }

  return result;
}

TableMetadata deserialize(const std::vector<char> &buffer) {
  size_t offset = 0;

  // Deserialize table_name
  uint32_t name_length = *reinterpret_cast<const uint32_t *>(
      buffer.data() + offset); // length can be negative
  offset += sizeof(uint32_t);
  std::string table_name(buffer.data() + offset,
                         buffer.data() + offset + name_length);
  offset += name_length;

  // Deserialize first_page_id
  page_id_t first_page_id =
      *reinterpret_cast<const page_id_t *>(buffer.data() + offset);
  offset += sizeof(page_id_t);

  // Deserialize schema - number of columns
  uint32_t num_columns =
      *reinterpret_cast<const uint32_t *>(buffer.data() + offset);
  offset += sizeof(uint32_t);

  // Deserialize each column
  std::vector<Column> columns;
  for (uint32_t i = 0; i < num_columns; ++i) {
    Column col;
    // Column name
    uint32_t col_name_length =
        *reinterpret_cast<const uint32_t *>(buffer.data() + offset);
    offset += sizeof(uint32_t);
    col.name = std::string(buffer.data() + offset,
                           buffer.data() + offset + col_name_length);
    offset += col_name_length;
    // Column type
    uint32_t type_val =
        *reinterpret_cast<const uint32_t *>(buffer.data() + offset);
    offset += sizeof(uint32_t);
    col.type = static_cast<Type>(type_val);
    // Column length
    col.length = *reinterpret_cast<const uint32_t *>(buffer.data() + offset);
    offset += sizeof(uint32_t);
    columns.push_back(col);
  }

  Schema schema(columns);
  return TableMetadata(table_name, schema, first_page_id);
}

std::vector<char> serialize(const Schema &schema,
                            const std::vector<std::string> &values) {

  uint32_t designed_size = schema.get_designed_size();
  std::vector<char> buffer(
      designed_size); // The buffer holds the largest possible record size based
                      // on the schema.

  const std::vector<Column> &columns =
      schema.get_columns(); // columns is a vector of <Column>.
  for (size_t i = 0; i < columns.size(); ++i) {
    if (columns[i].type == Type::INTEGER) {
      int val = std::stoi(values[i]); // stoi converts string to int
      // The vector::data() function in C++ returns a direct pointer to the
      // underlying contiguous array of elements managed by the std::vector.
      // This allows the vector's elements to be accessed or modified using raw
      // pointer arithmetic or passed to functions that expect a pointer to an
      // array.
      std::memset(buffer.data() + columns[i].offset, 0,
                  sizeof(int)); // Clear the 4 bytes we are about to write to.
      std::memcpy(buffer.data() + columns[i].offset, &val,
                  sizeof(int)); // Use &val to get the memory address of val,
                                // then copy 4 bytes to the buffer.
    } else if (columns[i].type == Type::VARCHAR) {

      // Clear the bytes we are about to write to, in case the new string is
      // shorter than the previous one.
      std::memset(buffer.data() + columns[i].offset, 0, columns[i].length);
      // This serves as a safeguard to prevent buffer overflow if the input
      // string is longer than the defined length for that column in the schema
      // so that max bytes it takes is the length defined in the schema.
      uint32_t varchar_bytes_to_copy =
          std::min((uint32_t)values[i].length(), columns[i].length);
      std::memcpy(buffer.data() + columns[i].offset, values[i].c_str(),
                  varchar_bytes_to_copy);
    }
  }
  return buffer;
}

Value deserialize(const char *buffer, Type type) {
  if (type == Type::INTEGER) {
    uint32_t val;
    memcpy(&val, buffer, sizeof(uint32_t));
    return Value(val);
  } else if (type == Type::VARCHAR) {
    std::string str(
        buffer); // Assumes the buffer is null-terminated. In our case, since we
                 // always clear the bytes before writing a new string, this
                 // should be safe because any unused bytes will be nulls.
    return Value(str);
  } else {
    throw std::runtime_error("Unknown type for deserialization");
  }
}

Value get_value(const std::vector<char> &raw_bytes, const Schema &schema,
                uint32_t col_num) {
  uint32_t offset = schema.get_column_offset(col_num);
  Type type = schema.get_column_type(col_num);
  return deserialize(raw_bytes.data() + offset, type);
}

void writeTableMetadata(Pager &pager, const std::string &table_name,
                        const Schema &schema) {

  page_id_t table_page_id = pager.allocate_new_page();

  auto metadata = TableMetadata(table_name, schema, table_page_id);

  std::vector<char> raw_bytes = serialize(metadata);

  size_t required_size = raw_bytes.size();

  page_id_t catalog_page_head = pager.get_file_header().catalog_page_head_;

  page_id_t inserted_at_page_id =
      insert_at_page_or_new(pager, catalog_page_head, raw_bytes);
}

std::vector<TableMetadata> readAllTableMetadata(const Pager &pager) {
  page_id_t catalog_page_id = pager.get_file_header().catalog_page_head_;
  std::vector<TableMetadata> result;

  while (catalog_page_id != INVALID_PAGE_ID) {
    std::vector<char> page_buffer = pager.read_page(catalog_page_id);
    PageHeader &page_header =
        *reinterpret_cast<PageHeader *>(page_buffer.data());
    for (auto slot_num = 0; slot_num < page_header.slot_count; ++slot_num) {
      auto record = pager.get_record_raw_bytes(catalog_page_id, slot_num);
      if (record.size() != 0) {
        auto schema = deserialize(record).schema;
        auto table_name = deserialize(record).table_name;
        auto first_page_id = deserialize(record).first_page_id;
        auto metadata = TableMetadata(table_name, schema, first_page_id);
        result.push_back(metadata);
      }
    }
    catalog_page_id = page_header.next_page_id;
  }

  return result;
}

TableMetadata getTableMetadata(const Pager &pager, std::string table_name) {
  auto all_table_metadata = readAllTableMetadata(pager);

  for (auto metadata : all_table_metadata) {
    if (metadata.table_name == table_name) {
      return metadata;
    }
  }
  throw std::runtime_error("Table does not exist or is corrupted.");
}

void writeRecord(Pager &pager, std::string &table_name, Schema &schema,
                 const std::vector<std::string> &records) {

  std::vector<char> raw_bytes = serialize(schema, records);

  size_t required_size = raw_bytes.size();

  TableMetadata metadata = getTableMetadata(pager, table_name);

  page_id_t page_head = metadata.first_page_id;

  page_id_t inserted_at_page_id =
      insert_at_page_or_new(pager, page_head, raw_bytes);
}

std::vector<std::vector<Value>> readTable(const Pager &pager,
                                            const std::string &table_name) {
  std::vector<std::vector<Value>> result = {};
  auto all_table_metadata = readAllTableMetadata(pager);

  page_id_t page_head = INVALID_PAGE_ID;
  Schema schema;

  for (auto metadata : all_table_metadata) {
    if (metadata.table_name == table_name) {
      page_head = metadata.first_page_id;
      schema = metadata.schema;
      break;
    }
  }

  while (page_head != INVALID_PAGE_ID) {
    std::vector<char> page_head_buffer = pager.read_page(page_head);

    PageHeader &page_head_header =
        *reinterpret_cast<PageHeader *>(page_head_buffer.data());

    auto slot_count = page_head_header.slot_count;

    for (auto slot_num = 0; slot_num < slot_count; ++slot_num) {
      std::vector<char> raw_bytes =
          pager.get_record_raw_bytes(page_head, slot_num);
      if (raw_bytes.empty()) continue; // skip deleted slots
      auto row = readRow(schema, raw_bytes);
      result.push_back(row);
    }

    page_head = page_head_header.next_page_id;
  }

  return result;
}

std::vector<Value> readRow(Schema &schema, const std::vector<char> &raw_bytes) {

  std::vector<Value> row = {};

  for (auto col_num = 0; col_num < schema.get_columns().size(); ++col_num) {
    Value value = get_value(raw_bytes, schema, col_num);
    row.push_back(value);
  }
  return row;
}

void deleteTableMetadata(Pager &pager, const std::string &table_name) {

  //
  page_id_t catalog_page_id = pager.get_file_header().catalog_page_head_;

  while (catalog_page_id != INVALID_PAGE_ID) {
    std::vector<char> page_buffer = pager.read_page(catalog_page_id);
    PageHeader &page_header =
        *reinterpret_cast<PageHeader *>(page_buffer.data());
    for (auto slot_num = 0; slot_num < page_header.slot_count; ++slot_num) {
      auto record = pager.get_record_raw_bytes(catalog_page_id, slot_num);
      if (deserialize(record).table_name == table_name) {
        pager.delete_record(catalog_page_id, slot_num);
        break;
      }
    }
    catalog_page_id = page_header.next_page_id;
  }

  // TableMetadata metadata = getTableMetadata(pager, table_name);
  // page_id_t page_head = metadata.first_page_id;
}

void truncateTable(Pager &pager, const std::string &table_name) {
  page_id_t page_head = getTableMetadata(pager, table_name).first_page_id;
  while (page_head != INVALID_PAGE_ID) {

    std::vector<char> page_head_buffer = pager.read_page(page_head);
    PageHeader &page_head_header =
        *reinterpret_cast<PageHeader *>(page_head_buffer.data());
    page_id_t next_page_id = page_head_header.next_page_id;

    pager.deallocate_page(page_head);

    page_head = next_page_id;
  }
}

page_id_t insert_at_page_or_new(Pager &pager, page_id_t page_head,
                                const std::vector<char> &raw_bytes) {

  page_id_t last_page_id = INVALID_PAGE_ID;
  size_t required_size = raw_bytes.size();

  while (page_head != INVALID_PAGE_ID) {
    std::vector<char> page_head_buffer = pager.read_page(page_head);

    PageHeader &page_head_header =
        *reinterpret_cast<PageHeader *>(page_head_buffer.data());

    auto free_space = page_head_header.get_free_space();

    if (free_space >= required_size) {
      pager.insert_record(raw_bytes, page_head);
      return page_head;
    }
    last_page_id = page_head;
    page_head = page_head_header.next_page_id;
  }
  if (last_page_id != INVALID_PAGE_ID) {
    std::vector<char> last_page_buffer = pager.read_page(last_page_id);
    PageHeader &last_page_header =
        *reinterpret_cast<PageHeader *>(last_page_buffer.data());

    page_id_t new_tail_page_id = pager.allocate_new_page();
    pager.insert_record(raw_bytes, new_tail_page_id);
    last_page_header.next_page_id = new_tail_page_id;

    std::vector<char> new_tail_page_buffer = pager.read_page(new_tail_page_id);
    PageHeader &new_tail_page_header =
        *reinterpret_cast<PageHeader *>(new_tail_page_buffer.data());
    new_tail_page_header.prev_page_id = last_page_id;

    pager.write_page(
        last_page_id,
        last_page_buffer.data()); // Don't forget to persist the change!
    pager.write_page(new_tail_page_id, new_tail_page_buffer.data());
    return new_tail_page_id;
  }
  return INVALID_PAGE_ID;
}

void peekPage(const Pager &pager, page_id_t page_id) {
  const FileHeader& file_header = pager.get_file_header();
  if (page_id == 0) { // For File header

    std::cout << "------File Header: ------" << std::endl;
    std::cout << "Magic Number: " << file_header.magic_number_ << std::endl;
    std::cout << "Version: " << file_header.version_ << std::endl;
    std::cout << "Catalog Page Head: " << file_header.catalog_page_head_
              << std::endl;
    std::cout << "Catalog Page Tail: " << file_header.catalog_page_tail_
              << std::endl;
    std::cout << "Page count: " << file_header.page_count_ << std::endl;
    std::cout << "Free Page Head: " << file_header.free_page_head_
              << std::endl;
    std::cout << "Free Page Tail: " << file_header.free_page_tail_
              << std::endl;

    return;
  } else if (page_id >= file_header.page_count_) {
    std::cout << "Page has not been allocated yet. " << std::endl;
    return;
  } else { // Statndard page
    std::vector<char> page_buffer = pager.read_page(page_id);
    PageHeader &page_header =
        *reinterpret_cast<PageHeader *>(page_buffer.data());

    std::cout << "------Page Header: ------" << std::endl;
    std::cout << "Magic Number: " << page_header.magic_number << std::endl;
    std::cout << "Previous Page: " << page_header.prev_page_id << std::endl;
    std::cout << "Next Page: " << page_header.next_page_id << std::endl;
    std::cout << "Lower Bound: " << page_header.lower_bound << std::endl;
    std::cout << "Upper Bound: " << page_header.upper_bound << std::endl;
    std::cout << "Slot Count: " << page_header.slot_count << std::endl;
    std::cout << "Deleted Count: " << page_header.deleted_count << std::endl;

    std::cout << "------Page Data: ------"
              << std::endl; // No cache, cannot get table name
    for (auto slot_num = 0; slot_num < page_header.slot_count; ++slot_num) {
    }
  }
}

void deleteRow(const Pager& pager, std::vector<Value>& target_row, const std::string& table_name) {
    
  auto all_table_metadata = readAllTableMetadata(pager);

  page_id_t page_head = INVALID_PAGE_ID;
  Schema schema;

  for (auto metadata : all_table_metadata) {
    if (metadata.table_name == table_name) {
      page_head = metadata.first_page_id;
      schema = metadata.schema;
      break;
    }
  }

  while (page_head != INVALID_PAGE_ID) {
    std::vector<char> page_head_buffer = pager.read_page(page_head);

    PageHeader &page_head_header =
        *reinterpret_cast<PageHeader *>(page_head_buffer.data());

    auto slot_count = page_head_header.slot_count;

    for (auto slot_num = 0; slot_num < slot_count; ++slot_num) {
      std::vector<char> raw_bytes =
          pager.get_record_raw_bytes(page_head, slot_num);
      if (raw_bytes.empty()) continue; // skip deleted slots
      auto row = readRow(schema, raw_bytes);
      //Must define == operator in Value class to compare
        if (row == target_row) { 
            pager.delete_record(page_head, slot_num); 
        }
    }
    page_head = page_head_header.next_page_id;
  }

}