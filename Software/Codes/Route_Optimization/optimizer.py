import math

# Mock Data: List of bins with (x, y) coordinates and current fill levels
bins = [
    {"id": "B01", "x": 10, "y": 10, "fill": 90},  # Critical
    {"id": "B02", "x": 12, "y": 15, "fill": 45},
    {"id": "B03", "x": 40, "y": 50, "fill": 85},  # Critical
    {"id": "B04", "x": 11, "y": 11, "fill": 60},  # Near B01 and > 50%
    {"id": "B05", "x": 90, "y": 90, "fill": 20},
]

# Constraints
CRITICAL_THRESHOLD = 80
SECONDARY_THRESHOLD = 50
MAX_DISTANCE_PICKUP = 5 # Units of distance to detour for a secondary bin

def calculate_distance(bin1, bin2):
    return math.sqrt((bin1["x"] - bin2["x"])**2 + (bin1["y"] - bin2["y"])**2)

def generate_route(bin_list):
    route = []
    critical_bins = [b for b in bin_list if b["fill"] >= CRITICAL_THRESHOLD]
    
    print(f"Detected {len(critical_bins)} critical bins.")

    # Simple Nearest Neighbor logic for the route
    current_pos = {"x": 0, "y": 0} # Depot location
    
    while critical_bins:
        # Find nearest critical bin
        nearest = min(critical_bins, key=lambda b: calculate_distance(current_pos, b))
        
        # Check for any "Secondary" bins on the way or nearby
        # (Rule: If >50% full and close to the critical bin, pick it up too)
        nearby_secondary = [
            b for b in bin_list 
            if b["fill"] >= SECONDARY_THRESHOLD 
            and b not in route 
            and b not in critical_bins
            and calculate_distance(nearest, b) < MAX_DISTANCE_PICKUP
        ]
        
        # Add secondary bins to route first if efficiently close
        for sb in nearby_secondary:
            route.append(sb)
            print(f"  -> Added Secondary Bin {sb['id']} (Fill: {sb['fill']}%) - On the way")
            
        # Add the critical bin
        route.append(nearest)
        critical_bins.remove(nearest)
        current_pos = nearest
        
    return route

# --- Main Execution ---
if __name__ == "__main__":
    print("--- Smart Waste Route Optimizer ---")
    final_route = generate_route(bins)
    
    print("\n--- Final Collection Sequence ---")
    print("START: DEPOT")
    for step, bin_obj in enumerate(final_route):
        print(f"STOP {step+1}: Bin {bin_obj['id']} (Fill: {bin_obj['fill']}%)")
    print("END: RETURN TO DEPOT")
