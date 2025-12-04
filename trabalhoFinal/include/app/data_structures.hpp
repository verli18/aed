#pragma once

#include <string>
#include <cstdint>
#include <chrono>
#include <random>

namespace app::ds {

// ============================================================================
// HASH-BASED ID GENERATOR
// ============================================================================
// Uses a combination of FNV-1a hash and base62 encoding to generate
// human-readable unique identifiers. This approach combines:
// - Content-based hashing for deterministic IDs
// - Timestamp component for uniqueness
// - Base62 encoding for compact, URL-safe representation
//
// Time Complexity: O(n) where n is input string length
// Space Complexity: O(1) for hash computation, O(k) for output where k is ID length
// ============================================================================

class HashIdGenerator {
public:
    // FNV-1a hash constants (64-bit)
    // These are prime numbers chosen for optimal distribution
    static constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
    
    // Base62 alphabet: 0-9, A-Z, a-z (alphanumeric, URL-safe)
    static constexpr const char* BASE62_ALPHABET = 
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    /// FNV-1a hash function - fast, good distribution, simple implementation
    /// Used by many hash tables and ID generators
    /// @param data Input string to hash
    /// @return 64-bit hash value
    static uint64_t fnv1a_hash(const std::string& data) {
        uint64_t hash = FNV_OFFSET_BASIS;
        
        for (unsigned char byte : data) {
            hash ^= byte;           // XOR with byte
            hash *= FNV_PRIME;      // Multiply by prime
        }
        
        return hash;
    }
    
    /// Encode a 64-bit value to base62 string
    /// Base62 is compact and uses only alphanumeric characters
    /// @param value Number to encode
    /// @param length Desired output length (will be padded/truncated)
    /// @return Base62 encoded string
    static std::string toBase62(uint64_t value, size_t length = 8) {
        std::string result;
        result.reserve(length);
        
        while (value > 0 && result.size() < length) {
            result = BASE62_ALPHABET[value % 62] + result;
            value /= 62;
        }
        
        // Pad with zeros if needed
        while (result.size() < length) {
            result = '0' + result;
        }
        
        return result.substr(0, length);
    }
    
    /// Generate a unique ID from content + timestamp
    /// Combines content hash with current time for uniqueness
    /// @param content The content to hash (e.g., "title|author")
    /// @param prefix Optional prefix for the ID (e.g., "BK" for books)
    /// @return Unique identifier string
    static std::string generateId(const std::string& content, const std::string& prefix = "") {
        // Get current timestamp in microseconds for uniqueness
        auto now = std::chrono::high_resolution_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()
        ).count();
        
        // Combine content hash with timestamp
        uint64_t contentHash = fnv1a_hash(content);
        uint64_t combinedHash = contentHash ^ (static_cast<uint64_t>(timestamp) * FNV_PRIME);
        
        // Generate base62 ID
        std::string id = toBase62(combinedHash, 8);
        
        return prefix.empty() ? id : prefix + "-" + id;
    }
    
    /// Generate a book ID from title and author
    static std::string generateBookId(const std::string& title, const std::string& author) {
        return generateId(title + "|" + author, "BK");
    }
    
    /// Generate a student ID from name and registration number
    static std::string generateStudentId(const std::string& name, const std::string& regNumber) {
        return generateId(name + "|" + regNumber, "ST");
    }
    
    /// Generate a loan ID from student and book IDs
    static std::string generateLoanId(const std::string& studentId, const std::string& bookId) {
        return generateId(studentId + "|" + bookId, "LN");
    }
};

// ============================================================================
// SIMPLE HASH TABLE IMPLEMENTATION
// ============================================================================
// A basic hash table using separate chaining for collision resolution.
// Included here to demonstrate understanding of hash-based data structures.
//
// Time Complexity:
//   - Insert: O(1) average, O(n) worst case
//   - Search: O(1) average, O(n) worst case
//   - Delete: O(1) average, O(n) worst case
// Space Complexity: O(n) where n is number of elements
// ============================================================================

template<typename K, typename V>
class HashTable {
private:
    struct Node {
        K key;
        V value;
        Node* next;
        
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };
    
    static constexpr size_t DEFAULT_CAPACITY = 16;
    static constexpr float LOAD_FACTOR_THRESHOLD = 0.75f;
    
    Node** buckets;
    size_t capacity;
    size_t size_;
    
    /// Hash function for keys
    size_t hash(const K& key) const {
        if constexpr (std::is_same_v<K, std::string>) {
            return HashIdGenerator::fnv1a_hash(key) % capacity;
        } else {
            return std::hash<K>{}(key) % capacity;
        }
    }
    
    /// Resize and rehash when load factor is exceeded
    void rehash() {
        size_t oldCapacity = capacity;
        Node** oldBuckets = buckets;
        
        capacity *= 2;
        buckets = new Node*[capacity]();
        size_ = 0;
        
        // Reinsert all elements
        for (size_t i = 0; i < oldCapacity; ++i) {
            Node* current = oldBuckets[i];
            while (current) {
                insert(current->key, current->value);
                Node* next = current->next;
                delete current;
                current = next;
            }
        }
        
        delete[] oldBuckets;
    }
    
public:
    HashTable() : capacity(DEFAULT_CAPACITY), size_(0) {
        buckets = new Node*[capacity]();
    }
    
    ~HashTable() {
        clear();
        delete[] buckets;
    }
    
    /// Insert or update a key-value pair
    void insert(const K& key, const V& value) {
        if (static_cast<float>(size_) / capacity >= LOAD_FACTOR_THRESHOLD) {
            rehash();
        }
        
        size_t index = hash(key);
        Node* current = buckets[index];
        
        // Check if key exists
        while (current) {
            if (current->key == key) {
                current->value = value;  // Update existing
                return;
            }
            current = current->next;
        }
        
        // Insert new node at head
        Node* newNode = new Node(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        ++size_;
    }
    
    /// Find value by key
    V* find(const K& key) {
        size_t index = hash(key);
        Node* current = buckets[index];
        
        while (current) {
            if (current->key == key) {
                return &current->value;
            }
            current = current->next;
        }
        
        return nullptr;
    }
    
    /// Check if key exists
    bool contains(const K& key) const {
        return const_cast<HashTable*>(this)->find(key) != nullptr;
    }
    
    /// Remove a key
    bool remove(const K& key) {
        size_t index = hash(key);
        Node* current = buckets[index];
        Node* prev = nullptr;
        
        while (current) {
            if (current->key == key) {
                if (prev) {
                    prev->next = current->next;
                } else {
                    buckets[index] = current->next;
                }
                delete current;
                --size_;
                return true;
            }
            prev = current;
            current = current->next;
        }
        
        return false;
    }
    
    /// Clear all entries
    void clear() {
        for (size_t i = 0; i < capacity; ++i) {
            Node* current = buckets[i];
            while (current) {
                Node* next = current->next;
                delete current;
                current = next;
            }
            buckets[i] = nullptr;
        }
        size_ = 0;
    }
    
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

} // namespace app::ds
