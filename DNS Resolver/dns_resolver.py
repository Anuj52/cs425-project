import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time

# We start with a list of root DNS servers to bootstrap the lookup.
# These servers are the backbone of the DNS system and will help us get referrals.
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

# This is the number of seconds we'll wait for a response before giving up on a query.
TIMEOUT = 3

def send_dns_query(server, domain):
    """
    This function sends a DNS query for an A record (IPv4 address) for a given domain.
    We ask the specified DNS server for the record and wait up to TIMEOUT seconds.
    
    Parameters:
      server (str): The IP address of the DNS server to query.
      domain (str): The domain we want to look up.
      
    Returns:
      A DNS response message if successful, or None if something goes wrong (like a timeout).
    """
    try:
        # Build our DNS query for the domain's A record.
        query = dns.message.make_query(domain, dns.rdatatype.A)
        # Send the query using UDP to the provided server, with a timeout.
        response = dns.query.udp(query, server, timeout=TIMEOUT)
        return response
    except Exception as e:
        print(f"[ERROR] Failed to query {server}: {e}")
        return None

def extract_next_nameservers(response):
    """
    When a DNS server doesn't have the final answer, it sends us referrals.
    This function grabs the nameserver (NS) records from the response's authority section
    and then resolves those NS hostnames to their IP addresses so we can query them next.
    
    Parameters:
      response (dns.message.Message): The DNS response that didn't give us the answer.
      
    Returns:
      A list of IP addresses for the next nameservers to try.
    """
    ns_ips = []    # This list will hold the IPs we find.
    ns_names = []  # We'll temporarily store the NS hostnames here.
    
    # Extract the NS hostnames from the authority section of the response.
    for rrset in response.authority:
        if rrset.rdtype == dns.rdatatype.NS:
            for rr in rrset:
                ns_name = rr.to_text()
                ns_names.append(ns_name)
                print(f"Extracted NS hostname: {ns_name}")
    
    # for each NS hostname, try to look up its A record (IP address).
    for ns_name in ns_names:
        try:
            answer = dns.resolver.resolve(ns_name, "A")
            for rdata in answer:
                ip = rdata.to_text()
                ns_ips.append(ip)
                print(f"Resolved NS {ns_name} -> {ip}")
        except Exception as e:
            print(f"[WARNING] Could not resolve NS {ns_name}: {e}")
    
    return ns_ips

def iterative_dns_lookup(domain):
    """
    This function performs an iterative DNS lookup starting at the root servers.
    It follows the chain from root to TLD to authoritative servers until it gets an answer.
    
    Parameters:
      domain (str): The domain we want to resolve.
    """
    print(f"[Iterative DNS Lookup] Resolving {domain}")

    # Start by using the root servers.
    next_ns_list = list(ROOT_SERVERS.keys())
    stage = "ROOT"  # We'll update this as we move from ROOT to TLD to AUTH.

    # Keep querying the next set of nameservers until we get an answer.
    while next_ns_list:
        ns_ip = next_ns_list.pop(0)
        response = send_dns_query(ns_ip, domain)

        if response:
            print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")
            
            # If the response contains an answer then print it.
            if response.answer:
                print(f"[SUCCESS] {domain} -> {response.answer[0][0]}")
                return
            
            # extract the next set of nameservers from this response.
            next_ns_list = extract_next_nameservers(response)
            
            # Move to the next stage of the lookup process.
            if stage == "ROOT":
                stage = "TLD"
            elif stage == "TLD":
                stage = "AUTH"
        else:
            print(f"[ERROR] Query failed for {stage} server {ns_ip}")
            return

    print("[ERROR] Resolution failed.")

def recursive_dns_lookup(domain):
    """
    This function uses the system's default recursive DNS resolver to get the answer.
    It hands the entire resolution process off to an external resolver (like Google DNS),
    which then returns the final answer.
    
    Parameters:
      domain (str): The domain we want to resolve.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
    try:
        # Use the system's default recursive resolver to get the answer.
        answer = dns.resolver.resolve(domain, "A")
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")
    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")

if __name__ == "__main__":
    import sys
    # Check the command-line arguments.
    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dns_server.py <iterative|recursive> <domain>")
        sys.exit(1)

    # Get the mode and domain from the command line arguments.
    mode = sys.argv[1]
    domain = sys.argv[2]
    start_time = time.time() # Record the start time.

    # Call the function based on the mode.
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)

    # Print out long the lookup took.
    print(f"Time taken: {time.time() - start_time:.3f} seconds")
