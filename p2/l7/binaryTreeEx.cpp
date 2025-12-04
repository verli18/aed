#include <iostream>
#include <queue>
using namespace std;

// Node structure
struct Node {
    int data;
    Node* left;
    Node* right;
    
    Node(int value) : data(value), left(nullptr), right(nullptr) {}
};

// Binary Tree class
class BinaryTree {
private:
    Node* root;
    
    // Helper: Recursive search
    bool searchRecursive(Node* node, int value) {
        if (node == nullptr) return false;
        if (node->data == value) return true;
        return searchRecursive(node->left, value) || searchRecursive(node->right, value);
    }
    
    // Helper: Inorder traversal (Left -> Root -> Right)
    void inorderRecursive(Node* node) {
        if (node != nullptr) {
            inorderRecursive(node->left);
            cout << node->data << " ";
            inorderRecursive(node->right);
        }
    }
    
    // Helper: Preorder traversal (Root -> Left -> Right)
    void preorderRecursive(Node* node) {
        if (node != nullptr) {
            cout << node->data << " ";
            preorderRecursive(node->left);
            preorderRecursive(node->right);
        }
    }
    
    // Helper: Postorder traversal (Left -> Right -> Root)
    void postorderRecursive(Node* node) {
        if (node != nullptr) {
            postorderRecursive(node->left);
            postorderRecursive(node->right);
            cout << node->data << " ";
        }
    }
    
    // Helper: Delete node
    Node* deleteRecursive(Node* current, int value) {
        if (current == nullptr) return nullptr;
        
        if (current->data == value) {
            // Case 1: Leaf node
            if (current->left == nullptr && current->right == nullptr) {
                delete current;
                return nullptr;
            }
            // Case 2: One child
            if (current->left == nullptr) {
                Node* temp = current->right;
                delete current;
                return temp;
            }
            if (current->right == nullptr) {
                Node* temp = current->left;
                delete current;
                return temp;
            }
            // Case 3: Two children - replace with successor
            Node* successor = findMin(current->right);
            current->data = successor->data;
            current->right = deleteRecursive(current->right, successor->data);
        } else {
            current->left = deleteRecursive(current->left, value);
            current->right = deleteRecursive(current->right, value);
        }
        return current;
    }
    
    Node* findMin(Node* node) {
        while (node->left != nullptr) node = node->left;
        return node;
    }
    
public:
    BinaryTree() : root(nullptr) {}
    
    // Insert node (level-order insertion for complete tree)
    void insert(int value) {
        Node* newNode = new Node(value);
        
        if (root == nullptr) {
            root = newNode;
            return;
        }
        
        queue<Node*> q;
        q.push(root);
        
        while (!q.empty()) {
            Node* current = q.front();
            q.pop();
            
            if (current->left == nullptr) {
                current->left = newNode;
                return;
            } else {
                q.push(current->left);
            }
            
            if (current->right == nullptr) {
                current->right = newNode;
                return;
            } else {
                q.push(current->right);
            }
        }
    }
    
    // Search for a value
    bool search(int value) {
        return searchRecursive(root, value);
    }
    
    // Delete a node
    void deleteNode(int value) {
        root = deleteRecursive(root, value);
    }
    
    // Traversals
    void inorder() {
        cout << "Inorder: ";
        inorderRecursive(root);
        cout << endl;
    }
    
    void preorder() {
        cout << "Preorder: ";
        preorderRecursive(root);
        cout << endl;
    }
    
    void postorder() {
        cout << "Postorder: ";
        postorderRecursive(root);
        cout << endl;
    }
    
    // Level-order traversal (BFS)
    void levelOrder() {
        if (root == nullptr) return;
        
        cout << "Level-order: ";
        queue<Node*> q;
        q.push(root);
        
        while (!q.empty()) {
            Node* current = q.front();
            q.pop();
            
            cout << current->data << " ";
            
            if (current->left != nullptr) q.push(current->left);
            if (current->right != nullptr) q.push(current->right);
        }
        cout << endl;
    }
};

int main() {
    BinaryTree tree;
    
    // Generate tree
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);
    tree.insert(5);
    tree.insert(6);
    
    // Traversals
    tree.inorder();
    tree.preorder();
    tree.postorder();
    tree.levelOrder();
    
    // Search
    cout << "Search 7: " << (tree.search(7) ? "Found" : "Not Found") << endl;
    cout << "Search 6: " << (tree.search(6) ? "Found" : "Not Found") << endl;
    
    // Delete
    tree.deleteNode(3);
    cout << "After deleting 3:" << endl;
    tree.inorder();
    
    return 0;
}
