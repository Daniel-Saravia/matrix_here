module bcd_7segmentVlog(
    input [3:0] bcd_in,
    output reg [6:0] seven_segment_out
);

// Combinational logic block for updating the output.
always @* begin
    seven_segment_out = decode_bcd_to_7segment(bcd_in);
end

// Function to convert BCD to 7-segment pattern
function [6:0] decode_bcd_to_7segment(input [3:0] bcd);
    begin
        case (bcd)
            4'b0000: decode_bcd_to_7segment = ~7'b0111111; // 0
            4'b0001: decode_bcd_to_7segment = ~7'b0000110; // 1
            4'b0010: decode_bcd_to_7segment = ~7'b1011011; // 2
            4'b0011: decode_bcd_to_7segment = ~7'b1001111; // 3
            4'b0100: decode_bcd_to_7segment = ~7'b1100110; // 4
            4'b0101: decode_bcd_to_7segment = ~7'b1101101; // 5
            4'b0110: decode_bcd_to_7segment = ~7'b1111101; // 6
            4'b0111: decode_bcd_to_7segment = ~7'b0000111; // 7
            4'b1000: decode_bcd_to_7segment = ~7'b1111111; // 8
            4'b1001: decode_bcd_to_7segment = ~7'b1101111; // 9
            default: decode_bcd_to_7segment = ~7'b1111111; // Default case if none of the above match
        endcase
    end
endfunction

endmodule
