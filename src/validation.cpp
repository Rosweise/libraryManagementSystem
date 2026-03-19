//  MIT License
//
//  Copyright (c) 2025 Dianna
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "./header/database.h"
#include <cctype>

// Input validation implementations
bool DatabaseManager::isValidUsername(const std::string& username) {
    // Username: 3-20 characters, alphanumeric and underscore only
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

bool DatabaseManager::isValidPassword(const std::string& password) {
    // Password: at least 8 characters
    if (password.length() < 8) {
        return false;
    }
    return true;
}

bool DatabaseManager::isValidStudentId(const std::string& studentId) {
    // Student ID: 5-15 characters, alphanumeric only
    if (studentId.length() < 5 || studentId.length() > 15) {
        return false;
    }
    for (char c : studentId) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

bool DatabaseManager::isValidISBN(const std::string& isbn) {
    // ISBN: non-empty, max 50 characters, allow digits, dashes, and spaces
    if (isbn.empty() || isbn.length() > 50) {
        return false;
    }
    for (char c : isbn) {
        if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-' && c != ' ') {
            return false;
        }
    }
    return true;
}

bool DatabaseManager::isValidRecoveryToken(const std::string& token) {
    // Recovery token: at least 4 characters
    if (token.length() < 4) {
        return false;
    }
    return true;
}
