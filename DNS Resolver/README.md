# DNS Resolver

A simple DNS resolution tool written in Python that supports both iterative and recursive lookups using the dnspython library.

## Overview

This project demonstrates how DNS resolution works by implementing two different methods:

- **Iterative DNS Lookup:**  
  Manually follows the referral chain starting from root servers to TLD servers and finally to authoritative servers to resolve a domain name.

- **Recursive DNS Lookup:**  
  Uses the system's default recursive resolver (like Google DNS or your ISP's DNS) to resolve a domain name.

## Prerequisites

- **Python 3.6 or above** (tested on Python 3.12.3)
- **dnspython Library**

## Installation and How to Run

1. **Install dnspython**  
   Open your terminal and run:
   ```bash
   pip3 install dnspython

2. **Run the Script:**
   - **Iterative Lookup:**
     ```bash
     python dns_server.py iterative example.com
     ```
   - **Recursive Lookup:**
     ```bash
     python dns_server.py recursive example.com
     ```

Replace `example.com` with the domain you want to resolve.
